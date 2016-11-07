/*
 *  Copyright (c) 2010 Luca Abeni
 *
 *  This is free software; see gpl-3.0.txt
 */

#define AUDIO_PAYLOAD_HEADER_SIZE 1 + 1 + 2 + 2 + 1 // 1 Frame type + 1 channels 2 sample rate + 2 frame size + 1 number of frames
#define VIDEO_PAYLOAD_HEADER_SIZE 1 + 2 + 2 + 2 + 2 + 1 // 1 Frame type + 2 width + 2 height + 2 frame rate num + 2 frame rate den + 1 number of frames
#define FRAME_HEADER_SIZE (3 + 4 + 1)	// 3 Frame size + 4 PTS + 1 DeltaTS
#define UDP_PAYLOAD_HEADER_SIZE (2 + 1)   // 2 size + 1 stream
#define RTP_PAYLOAD_PER_PKT_HEADER_SIZE (2 + 1)   // 2 size + 1 stream

static inline void frame_header_parse(const uint8_t *data, int *size, int64_t *pts, int64_t *dts)
{
  int i;

  *size = 0;
/* FIXME: Maybe this can use int_coding? */

  for (i = 0; i < 3; i++) {
    *size = *size << 8;
    *size |= data[i];
  }
  *dts = int_rcpy(data + 3);
  if (data[7] != 255) {
    *pts = *dts + data[7];
  } else {
    *pts = -1;
  }
}

static inline void audio_payload_header_parse(const uint8_t *data, uint8_t *codec, uint8_t *ch, int *sr, int *fs)
{
  *codec = data[0];
  *ch = data[1];
  *sr = int16_rcpy(data + 2);
  *fs = int16_rcpy(data + 4);
}

static inline void video_payload_header_parse(const uint8_t *data, uint8_t *codec, int *width, int *height, int *frame_rate_n, int *frame_rate_d)
{
  *codec = data[0];
  *width = int16_rcpy(data + 1);
  *height = int16_rcpy(data + 3);
  *frame_rate_n = int16_rcpy(data + 5);
  *frame_rate_d = int16_rcpy(data + 7);
}

static inline void audio_payload_header_write(uint8_t *data, uint8_t codec, unsigned int channels, unsigned int sample_rate, unsigned int frame_size)
{
  data[0] = codec;
  data[1] = channels;
  int16_cpy(data + 2, sample_rate);
  int16_cpy(data + 4, frame_size);
}

static inline void video_payload_header_write(uint8_t *data, uint8_t codec, int width, int height, int num, int den)
{
  data[0] = codec;
  int16_cpy(data + 1, width);
  int16_cpy(data + 3, height);
  int16_cpy(data + 5, num);
  int16_cpy(data + 7, den);
}

static inline void frame_header_write(uint8_t *data, int size, int32_t pts, int32_t dts)
{
  data[0] = size >> 16;
  data[1] = size >> 8;
  data[2] = size & 0xFF;
  int_cpy(data + 3, dts);
  if (pts != -1) {
    data[7] = (pts - dts) & 0xFF;
  } else {
    data[7] = 255;
  }
}

static inline void udp_payload_header_parse(uint8_t *data, int *size, int *stream)
{
  *size = int16_rcpy(data);
  *stream = data[2]; 
}

static inline void udp_payload_header_write(uint8_t *data, int size, uint8_t stream)
{
  int16_cpy(data, size);
  data[2] = stream;
}

/* RTP chunk payload format:
   0    1    2    3
   +----+----+----+-------------
   |  size   |s_id| data (RTP packet with header)
   +---------+----+-------------
   |  ...
   +---------+------------------
   |  size   |s_id| data ...
   +---------+----+-------------
   size: size of the following `data` field (16 bits)
   s_id: stream identifier (8 bits)
   There's no need to carry total number of packets, because total length
   is known at dechunkising time
 */

static inline void rtp_payload_per_pkt_header_set(uint8_t *data, uint16_t size, uint8_t stream) {
  int16_cpy(data, size);
  *(data + 2) = stream;
}

static inline void rtp_payload_per_pkt_header_parse(uint8_t *data, int *size, int *stream)
{
  *size = int16_rcpy(data);
  *stream = *(data + 2);
}
