#ifndef FILTERS_H_
#define FILTERS_H_

#ifdef __cplusplus
extern "C" {
#endif

int init_filters();
void cleanup_filters();

void apply_filter(unsigned int dest, unsigned int src, int xsz, int ysz, unsigned int sdr);

void edge_detect(unsigned int dest, unsigned int src, int xsz, int ysz);
void gauss_blur(unsigned int dest, unsigned int src, int xsz, int ysz, float stddev);

#ifdef __cplusplus
}
#endif

#endif	/* FILTERS_H_ */
