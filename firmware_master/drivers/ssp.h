#ifndef SSP_H_
#define SSP_H_

void sspInit(void);
void sspSendByte(uint8_t buf);
void sspSend(const uint8_t *buf, uint32_t length);
void sspReceive(uint8_t *buf, uint32_t length);
void sspSendReceive(uint8_t *buf, uint32_t length);

#endif
