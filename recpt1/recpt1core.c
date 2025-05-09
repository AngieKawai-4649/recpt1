#include <stdio.h>
#include <stdlib.h>
#include "recpt1core.h"
#include "pt1_dev.h"


#define ISDB_T_NODE_LIMIT 24        // 32:ARIB limit 24:program maximum
#define ISDB_T_SLOT_LIMIT 8

/* globals */
boolean f_exit = FALSE;
/* warning 除去の為bs_channel_buf 8->32 byteに変更 AngieKawai-4649 2025-02-08 */
char  bs_channel_buf[32];

int
close_tuner(thread_data *tdata)
{
	int rv = 0;

	if(tdata->tfd == -1)
		return rv;

	int current_type;
	if(tdata->table.tuning_space == SPI_UHF || tdata->table.tuning_space == SPI_CATV){
		current_type = CHTYPE_GROUND;
	}else{
		current_type = CHTYPE_SATELLITE;
	}

	if(current_type == CHTYPE_SATELLITE) {
		if(ioctl(tdata->tfd, LNB_DISABLE, 0) < 0) {
			rv = 1;
		}
	}
	close(tdata->tfd);
	tdata->tfd = -1;

	return rv;
}

float
getsignal_isdb_s(int signal)
{
	/* apply linear interpolation */
	static const float afLevelTable[] = {
		24.07f,    // 00    00    0        24.07dB
		24.07f,    // 10    00    4096     24.07dB
		18.61f,    // 20    00    8192     18.61dB
		15.21f,    // 30    00    12288    15.21dB
		12.50f,    // 40    00    16384    12.50dB
		10.19f,    // 50    00    20480    10.19dB
		8.140f,    // 60    00    24576    8.140dB
		6.270f,    // 70    00    28672    6.270dB
		4.550f,    // 80    00    32768    4.550dB
		3.730f,    // 88    00    34816    3.730dB
		3.630f,    // 88    FF    35071    3.630dB
		2.940f,    // 90    00    36864    2.940dB
		1.420f,    // A0    00    40960    1.420dB
		0.000f     // B0    00    45056    -0.01dB
	};

	unsigned char sigbuf[4];
	memset(sigbuf, '\0', sizeof(sigbuf));
	sigbuf[0] =  (((signal & 0xFF00) >> 8) & 0XFF);
	sigbuf[1] =  (signal & 0xFF);

	/* calculate signal level */
	if(sigbuf[0] <= 0x10U) {
		/* clipped maximum */
		return 24.07f;
	}
	else if (sigbuf[0] >= 0xB0U) {
		/* clipped minimum */
		return 0.0f;
	}
	else {
		/* linear interpolation */
		const float fMixRate =
			(float)(((unsigned short)(sigbuf[0] & 0x0FU) << 8) |
					(unsigned short)sigbuf[0]) / 4096.0f;
		return afLevelTable[sigbuf[0] >> 4] * (1.0f - fMixRate) +
			afLevelTable[(sigbuf[0] >> 4) + 0x01U] * fMixRate;
	}
}

void
calc_cn(int fd, int type, boolean use_bell)
{
	int     rc;
	double  P;
	double  CNR;
	int bell = 0;

	if(ioctl(fd, GET_SIGNAL_STRENGTH, &rc) < 0) {
		fprintf(stderr, "Tuner Select Error\n");
		return ;
	}

	if(type == CHTYPE_GROUND) {
		P = log10(5505024/(double)rc) * 10;
		CNR = (0.000024 * P * P * P * P) - (0.0016 * P * P * P) +
					(0.0398 * P * P) + (0.5491 * P)+3.0965;
	}
	else {
		CNR = getsignal_isdb_s(rc);
	}

	if(use_bell) {
		if(CNR >= 30.0)
			bell = 3;
		else if(CNR >= 15.0 && CNR < 30.0)
			bell = 2;
		else if(CNR < 15.0)
			bell = 1;
		fprintf(stderr, "\rC/N = %fdB ", CNR);
		do_bell(bell);
	}
	else {
		fprintf(stderr, "\rC/N = %fdB", CNR);
	}
}

#define CHANNEL_INFORMATION(mi,ch) { \
	CHANNEL_INFO wc; \
	fprintf(stderr, "%s\n", mi); \
	fprintf(stderr, "Channel  TSID       SID   Name\n"); \
	fprintf(stderr, "------------------------------\n"); \
	for(uint8_t i = 0; ; i++){ \
		if(search_tuning_space(ch, i, &wc) == CH_RETURN_FOUND){ \
			fprintf(stderr, "%-7s  0x%04x   %5d   %s\n", wc.channel_key, wc.tsid, wc.sid, wc.ch_name); \
		}else{ \
			break; \
		} \
	} \
}


void
show_channels(void)
{

	fprintf(stderr, "Available Channels:\n");

	print_channel_cnt(stderr);

	CHANNEL_INFORMATION("[地デジ]", SPI_UHF);
	CHANNEL_INFORMATION("[CATV]",   SPI_CATV);
	CHANNEL_INFORMATION("[BS]",     SPI_BS);
	CHANNEL_INFORMATION("[CS]",     SPI_CS);
}


#if 0
int
parse_time(char *rectimestr, thread_data *tdata)
{
	/* indefinite */
	if(!strcmp("-", rectimestr)) {
		tdata->indefinite = TRUE;
		tdata->recsec = -1;
		return 0;
	}
	/* colon */
	else if(strchr(rectimestr, ':')) {
		int n1, n2, n3;
		if(sscanf(rectimestr, "%d:%d:%d", &n1, &n2, &n3) == 3)
			tdata->recsec = n1 * 3600 + n2 * 60 + n3;
		else if(sscanf(rectimestr, "%d:%d", &n1, &n2) == 2)
			tdata->recsec = n1 * 3600 + n2 * 60;
		else
			return 1;
		return 0;
	}
	/* HMS */
	else {
		char *tmpstr;
		char *p1, *p2;
		int  flag;

		if( *rectimestr == '-' ){
			rectimestr++;
			flag = 1;
		}else
			flag = 0;
		tmpstr = strdup(rectimestr);
		p1 = tmpstr;
		while(*p1 && !isdigit(*p1))
			p1++;

		/* hour */
		if((p2 = strchr(p1, 'H')) || (p2 = strchr(p1, 'h'))) {
			*p2 = '\0';
			tdata->recsec += atoi(p1) * 3600;
			p1 = p2 + 1;
			while(*p1 && !isdigit(*p1))
				p1++;
		}

		/* minute */
		if((p2 = strchr(p1, 'M')) || (p2 = strchr(p1, 'm'))) {
			*p2 = '\0';
			tdata->recsec += atoi(p1) * 60;
			p1 = p2 + 1;
			while(*p1 && !isdigit(*p1))
				p1++;
		}

		/* second */
		tdata->recsec += atoi(p1);
		if( flag )
			*recsec *= -1;

		free(tmpstr);
		return 0;
	} /* else */

	return 1; /* unsuccessful */
}
#endif

int
parse_time(char *rectimestr, int *recsec)
{
	/* indefinite */
	if(!strcmp("-", rectimestr)) {
		*recsec = -1;
		return 0;
	}
	/* colon */
	else if(strchr(rectimestr, ':')) {
		int n1, n2, n3;
		if(sscanf(rectimestr, "%d:%d:%d", &n1, &n2, &n3) == 3)
			*recsec = n1 * 3600 + n2 * 60 + n3;
		else if(sscanf(rectimestr, "%d:%d", &n1, &n2) == 2)
			*recsec = n1 * 3600 + n2 * 60;
		else
			return 1; /* unsuccessful */

		return 0;
	}
	/* HMS */
	else {
		char *tmpstr;
		char *p1, *p2;
		int  flag;

		if( *rectimestr == '-' ){
			rectimestr++;
			flag = 1;
		}else
			flag = 0;
		tmpstr = strdup(rectimestr);
		p1 = tmpstr;
		while(*p1 && !isdigit(*p1))
			p1++;

		/* hour */
		if((p2 = strchr(p1, 'H')) || (p2 = strchr(p1, 'h'))) {
			*p2 = '\0';
			*recsec += atoi(p1) * 3600;
			p1 = p2 + 1;
			while(*p1 && !isdigit(*p1))
				p1++;
		}

		/* minute */
		if((p2 = strchr(p1, 'M')) || (p2 = strchr(p1, 'm'))) {
			*p2 = '\0';
			*recsec += atoi(p1) * 60;
			p1 = p2 + 1;
			while(*p1 && !isdigit(*p1))
				p1++;
		}

		/* second */
		*recsec += atoi(p1);
		if( flag )
			*recsec *= -1;

		free(tmpstr);

		return 0;
	} /* else */

	return 1; /* unsuccessful */
}

void
do_bell(int bell)
{
	int i;
	for(i=0; i < bell; i++) {
		fprintf(stderr, "\a");
		usleep(400000);
	}
}

/* from checksignal.c */
int
tune(char *channel, thread_data *tdata, char *device)
{
	char **tuner;
	int num_devs;
	int lp;
	FREQUENCY freq;
	int ret;

	/* get channel */
	ret = search_channel_key(channel, 0, &(tdata->table));
	if(ret == CH_RETURN_NOTFOUND){
		fprintf(stderr, "Invalid Channel: %s\n", channel);
		return 1;
	}

	freq.frequencyno = tdata->table.pt_ch;
	freq.slot = tdata->table.l_tsid;

	/* open tuner */
	/* case 1: specified tuner device */
	if(device) {
		tdata->tfd = open(device, O_RDONLY);
		if(tdata->tfd < 0) {
			fprintf(stderr, "Cannot open tuner device: %s\n", device);
			return 1;
		}

		/* power on LNB */
		int current_type;
		if(tdata->table.tuning_space == SPI_UHF || tdata->table.tuning_space == SPI_CATV){
			current_type = CHTYPE_GROUND;
		}else{
			current_type = CHTYPE_SATELLITE;
		}

		if(current_type == CHTYPE_SATELLITE) {
			if(ioctl(tdata->tfd, LNB_ENABLE, tdata->lnb) < 0) {
				fprintf(stderr, "Power on LNB failed: %s\n", device);
			}
		}

		/* tune to specified channel */
		while(ioctl(tdata->tfd, SET_CHANNEL, &freq) < 0) {
			if(tdata->tune_persistent) {
				if(f_exit) {
					close_tuner(tdata);
					return 1;
				}
				fprintf(stderr, "No signal. Still trying: %s\n", device);
			}
			else {
				close(tdata->tfd);
				fprintf(stderr, "Cannot tune to the specified channel: %s\n", device);
				return 1;
			}
		}

		fprintf(stderr, "device = %s\n", device);
	}
	else {
		/* case 2: loop around available devices */
		int current_type;
		if(tdata->table.tuning_space == SPI_UHF || tdata->table.tuning_space == SPI_CATV){
			current_type = CHTYPE_GROUND;
		}else{
			current_type = CHTYPE_SATELLITE;
		}

		if(current_type == CHTYPE_SATELLITE) {
			tuner = bsdev;
			num_devs = NUM_BSDEV;
		}
		else {
			tuner = isdb_t_dev;
			num_devs = NUM_ISDB_T_DEV;
		}

		for(lp = 0; lp < num_devs; lp++) {
			int count = 0;

			tdata->tfd = open(tuner[lp], O_RDONLY);
			if(tdata->tfd >= 0) {
				/* power on LNB */
				if(current_type == CHTYPE_SATELLITE) {
					if(ioctl(tdata->tfd, LNB_ENABLE, tdata->lnb) < 0) {
						fprintf(stderr, "Warning: Power on LNB failed: %s\n", tuner[lp]);
					}
				}

				/* tune to specified channel */
				if(tdata->tune_persistent) {
					while(ioctl(tdata->tfd, SET_CHANNEL, &freq) < 0 &&
						  count < MAX_RETRY) {
						if(f_exit) {
							close_tuner(tdata);
							return 1;
						}
						fprintf(stderr, "No signal. Still trying: %s\n", tuner[lp]);
						count++;
					}

					if(count >= MAX_RETRY) {
						close_tuner(tdata);
						count = 0;
						continue;
					}
				} /* tune_persistent */
				else {
					if(ioctl(tdata->tfd, SET_CHANNEL, &freq) < 0) {
						close(tdata->tfd);
						tdata->tfd = -1;
						continue;
					}
				}

				if(tdata->tune_persistent)
					fprintf(stderr, "device = %s\n", tuner[lp]);
				break; /* found suitable tuner */
			}
		}

		/* all tuners cannot be used */
		if(tdata->tfd < 0) {
			fprintf(stderr, "Cannot tune to the specified channel\n");
			return 1;
		}
	}

	if(!tdata->tune_persistent) {
		/* show signal strength */
		int current_type;
		if(tdata->table.tuning_space == SPI_UHF || tdata->table.tuning_space == SPI_CATV){
			current_type = CHTYPE_GROUND;
		}else{
			current_type = CHTYPE_SATELLITE;
		}

		calc_cn(tdata->tfd, current_type, FALSE);
	}

	return 0; /* success */
}

