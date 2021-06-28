/* ------------------------------------------------------------------
 * Copyright (C) 2011 Martin Storsjo
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 * -------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>

#if defined(_MSC_VER)
#include <getopt.h>
#else
#include <unistd.h>
#endif

#include <stdlib.h>
#include "/home/xuzhiyin/project/test/fdk-aac-2.0.1/output/include/aacenc_lib.h"
#define D printf
void usage(const char* name) {
	fprintf(stderr, "%s [-r bitrate] [-t aot] [-a afterburner] [-s sbr] [-v vbr] in.wav out.aac\n", name);
	fprintf(stderr, "Supported AOTs:\n");
	fprintf(stderr, "\t2\tAAC-LC\n");
	fprintf(stderr, "\t5\tHE-AAC\n");
	fprintf(stderr, "\t29\tHE-AAC v2\n");
	fprintf(stderr, "\t23\tAAC-LD\n");
	fprintf(stderr, "\t39\tAAC-ELD\n");
}

int main(int argc, char *argv[]) {
	int bitrate = 64000;
	int ch;
	const char *PCM_FILE = "./enc.pcm", *AAC_FILE = "enc.aac";
    int pcm_fd = -1, aac_fd = -1;
	int sample_rate, channels, bits_per_sample;
	int input_size;
	uint8_t* input_buf;
	int16_t* convert_buf;
	int aot = 2;
	int afterburner = 1;
	int eld_sbr = 0;
	int vbr = 0;
	HANDLE_AACENCODER handle;
	CHANNEL_MODE mode;
	AACENC_InfoStruct info = { 0 };


    channels = 1;
    aot = 2;
    sample_rate = 16000;
    mode = MODE_1;
    vbr = 0;
    bitrate = 16000;
    afterburner = 1;

	if (aacEncOpen(&handle, 0, channels) != AACENC_OK) {
		fprintf(stderr, "Unable to open encoder\n");
		return 1;
	}
	if (aacEncoder_SetParam(handle, AACENC_AOT, aot) != AACENC_OK) {
		fprintf(stderr, "Unable to set the AOT\n");
		return 1;
	}
	if (aot == 39 && eld_sbr) {
		if (aacEncoder_SetParam(handle, AACENC_SBR_MODE, 1) != AACENC_OK) {
			fprintf(stderr, "Unable to set SBR mode for ELD\n");
			return 1;
		}
	}
	if (aacEncoder_SetParam(handle, AACENC_SAMPLERATE, sample_rate) != AACENC_OK) {
		fprintf(stderr, "Unable to set the AOT\n");
		return 1;
	}
	if (aacEncoder_SetParam(handle, AACENC_CHANNELMODE, mode) != AACENC_OK) {
		fprintf(stderr, "Unable to set the channel mode\n");
		return 1;
	}
	if (aacEncoder_SetParam(handle, AACENC_CHANNELORDER, 1) != AACENC_OK) {
		fprintf(stderr, "Unable to set the wav channel order\n");
		return 1;
	}
	if (vbr) {
		if (aacEncoder_SetParam(handle, AACENC_BITRATEMODE, vbr) != AACENC_OK) {
			fprintf(stderr, "Unable to set the VBR bitrate mode\n");
			return 1;
		}
	} else {
		if (aacEncoder_SetParam(handle, AACENC_BITRATE, bitrate) != AACENC_OK) {
			fprintf(stderr, "Unable to set the bitrate\n");
			return 1;
		}
	}
	if (aacEncoder_SetParam(handle, AACENC_TRANSMUX, TT_MP4_ADTS) != AACENC_OK) {
		fprintf(stderr, "Unable to set the ADTS transmux\n");
		return 1;
	}
	if (aacEncoder_SetParam(handle, AACENC_AFTERBURNER, afterburner) != AACENC_OK) {
		fprintf(stderr, "Unable to set the afterburner mode\n");
		return 1;
	}
	if (aacEncEncode(handle, NULL, NULL, NULL, NULL) != AACENC_OK) {
		fprintf(stderr, "Unable to initialize the encoder\n");
		return 1;
	}
	if (aacEncInfo(handle, &info) != AACENC_OK) {
		fprintf(stderr, "Unable to get the encoder info\n");
		return 1;
	}

	pcm_fd = open(PCM_FILE, O_RDONLY);
	if (pcm_fd < 0) {
		perror(PCM_FILE);
		return -1;
	}

	input_size = channels*2*info.frameLength;
	input_buf = (uint8_t*) malloc(input_size);
	convert_buf = (int16_t*) malloc(input_size);
    D("frame len:%d\n", info.frameLength);
    D("input size:%d\n", input_size);

	while (1) {
		AACENC_BufDesc in_buf = { 0 }, out_buf = { 0 };
		AACENC_InArgs in_args = { 0 };
		AACENC_OutArgs out_args = { 0 };
		int in_identifier = IN_AUDIO_DATA;
		int in_size, in_elem_size;
		int out_identifier = OUT_BITSTREAM_DATA;
		int out_size, out_elem_size;
		int read_cnt, i;
		void *in_ptr, *out_ptr;
		uint8_t outbuf[20480];
		AACENC_ERROR err;

        read_cnt = read(pcm_fd, input_buf, input_size);
        if (read_cnt == 0) {
            D("read_cnt error\n");
            return -1;
        }
        D("infact read_cnt data:%d\n", read_cnt);
		for (i = 0; i < read_cnt/2; i++) {
			const uint8_t* in = &input_buf[2*i];
			convert_buf[i] = in[0] | (in[1] << 8);
		}
		in_ptr = convert_buf;
		in_size = read_cnt;
		in_elem_size = 2;

		in_args.numInSamples = read_cnt <= 0 ? -1 : read_cnt/2;
		in_buf.numBufs = 1;
		in_buf.bufs = &in_ptr;
		in_buf.bufferIdentifiers = &in_identifier;
		in_buf.bufSizes = &in_size;
		in_buf.bufElSizes = &in_elem_size;

		out_ptr = outbuf;
		out_size = sizeof(outbuf);
		out_elem_size = 1;
		out_buf.numBufs = 1;
		out_buf.bufs = &out_ptr;
		out_buf.bufferIdentifiers = &out_identifier;
		out_buf.bufSizes = &out_size;
		out_buf.bufElSizes = &out_elem_size;

		if ((err = aacEncEncode(handle, &in_buf, &out_buf, &in_args, &out_args)) != AACENC_OK) {
			if (err == AACENC_ENCODE_EOF)
				break;
			fprintf(stderr, "Encoding failed\n");
			return 1;
		}
        D("parse out_size:%d\n", out_args.numOutBytes);
		if (out_args.numOutBytes == 0) {
            D("parse error\n");
			continue;
        }
		//fwrite(outbuf, 1, out_args.numOutBytes, out);
	}
	free(input_buf);
	free(convert_buf);
	close(pcm_fd);
	aacEncClose(&handle);

	return 0;
}

