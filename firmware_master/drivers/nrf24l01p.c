#include <drivers/nrf24l01p.h>
#include <main.h>
#include "drivers/ssp.h"

#define DEFAULT_SPEED R_RF_SETUP_DR_2M


#define CE_HIGH()    (LPC_GPIO2->FIOPIN |=  (1 << 2))
#define CE_LOW()    (LPC_GPIO2->FIOPIN &= ~(1 << 2))
#define CS_HIGH()    (LPC_GPIO2->FIOPIN |=  (1 << 6))
#define CS_LOW()    (LPC_GPIO2->FIOPIN &= ~(1 << 6))


void nrf_cmd(uint8_t cmd)
{
	CS_LOW();
	sspSendByte (cmd);
	CS_HIGH();
};


void nrf_write_reg(const uint8_t reg, const uint8_t val)
{
	CS_LOW();
	sspSendByte(C_W_REGISTER | reg);
	sspSendByte(val);
	CS_HIGH();
};


void nrf_write_long(const uint8_t cmd, int len, const uint8_t* data)
{
	CS_LOW();
	sspSendByte(cmd);
	sspSend(data,len);
	CS_HIGH();
};

#define nrf_write_reg_long(reg, len, data) \
	nrf_write_long(C_W_REGISTER|(reg), len, data)


void nrf_set_rx_mac(int pipe, int rxlen, int maclen, const uint8_t * mac)
{
	nrf_write_reg(R_RX_PW_P0+pipe,rxlen);
	nrf_write_reg_long(R_RX_ADDR_P0+pipe,maclen,mac);
};

void nrf_set_tx_mac(int maclen, const uint8_t * mac)
{
	nrf_write_reg_long(R_TX_ADDR,maclen,mac);
};


void nrf_set_channel(int channel)
{
	nrf_write_reg(R_RF_CH, channel);
};

void nrf_config_set(nrfconfig config)
{
	nrf_write_reg(R_SETUP_AW,R_SETUP_AW_5);

	nrf_set_channel(config->channel);

	for(int i=0;i<config->nrmacs;i++){
		nrf_write_reg(R_RX_PW_P0+i,config->maclen[i]);
		if(i==0){
			nrf_write_reg_long(R_RX_ADDR_P0,5,config->mac0);
		}else if(i==1){
			nrf_write_reg_long(R_RX_ADDR_P1,5,config->mac1);
		}else if(i>1){
			nrf_write_reg_long(R_RX_ADDR_P0+i,1,config->mac2345+i-2);
		};
	};

	nrf_write_reg_long(R_TX_ADDR,5,config->txmac);

	nrf_write_reg(R_EN_RXADDR,(1<<config->nrmacs)-1);
};

void nrf_set_strength(unsigned char strength){

	if(strength>3)
		strength=3;
	nrf_write_reg(R_RF_SETUP,DEFAULT_SPEED|(strength<<1));
};


void nrf_init() 
{
	//ce
	LPC_GPIO2->FIODIR |= (1<<2);
	//cs_rad
	LPC_GPIO2->FIODIR |= (1<<6);

	CE_LOW();
	delay_ms(11);

	// Setup for nrf24l01+
	// power up takes 1.5ms - 3.5ms (depending on crystal)
	CS_LOW();
	nrf_write_reg(R_CONFIG,
			R_CONFIG_PRIM_RX| // Receive mode
			R_CONFIG_PWR_UP|  // Power on
			R_CONFIG_EN_CRC   // CRC on, single byte
			);

	nrf_write_reg(R_EN_AA, 0); // Disable Enhanced ShockBurst;

	// Set speed / strength
	nrf_write_reg(R_RF_SETUP,DEFAULT_SPEED|R_RF_SETUP_RF_PWR_3);

	// Clear MAX_RT, just in case.
	nrf_write_reg(R_STATUS,R_STATUS_MAX_RT);

	//    CS_HIGH();

};



char snd_pkt_no_crc(int size, uint8_t * pkt)
{
	if(size > MAX_PKT)
		size=MAX_PKT;

	nrf_write_reg(R_CONFIG,
			R_CONFIG_PWR_UP|  // Power on
			R_CONFIG_EN_CRC   // CRC on, single byte
			);

	CS_LOW();
	sspSendByte(C_W_TX_PAYLOAD);
	sspSend(pkt,size);
	CS_HIGH();
	CS_HIGH();
	CS_HIGH();

	CE_HIGH();
	delay_ms(1); // Send it.  (only needs >10ys, i think)
	CE_LOW();

	return 0;
	//    return nrf_cmd_status(C_NOP);
};




void nrf_read_long(const uint8_t cmd, int len, uint8_t* data)
{
	CS_LOW();
	sspSendByte(cmd);
	for(int i=0;i<len;i++)
		data[i] = 0x00;
	sspSendReceive(data,len);
	CS_HIGH();
};


// High-Level:
void nrf_rcv_pkt_start(void)
{

	nrf_write_reg(R_CONFIG,
			R_CONFIG_PRIM_RX| // Receive mode
			R_CONFIG_PWR_UP|  // Power on
			R_CONFIG_EN_CRC   // CRC on, single byte
			);

	nrf_cmd(C_FLUSH_RX);
	nrf_write_reg(R_STATUS,0);

	CE_HIGH();
};

void nrf_read_pkt(int len, uint8_t* data)
{
	CS_LOW();
	sspSendByte(C_R_RX_PAYLOAD);
	sspReceive(data,len);
	CS_HIGH();
};


uint8_t nrf_cmd_status(uint8_t cmd)
{
	CS_LOW();
	sspSendReceive(&cmd, 1);
	CS_HIGH();
	return cmd;
};

int nrf_rcv_pkt_poll(int maxsize, uint8_t * pkt)
{
	uint8_t len;
	uint8_t status=0;

	for(int i=0;i<maxsize;i++) pkt[i] = 0x00; // Sanity: clear packet buffer

	status =nrf_cmd_status(C_NOP);

	if((status & R_STATUS_RX_P_NO) == R_STATUS_RX_FIFO_EMPTY){
		if( (status & R_STATUS_RX_DR) == R_STATUS_RX_DR){
			nrf_write_reg(R_STATUS,R_STATUS_RX_DR);
		};
		return 0;
	};

	nrf_read_long(C_R_RX_PL_WID,1,&len);

	nrf_write_reg(R_STATUS,R_STATUS_RX_DR);
	if(len>32 || len==0){
		return -2; // no packet error
	};

	if(len>maxsize){
		return -1; // packet too large
	};

	nrf_read_pkt(len,pkt);

	return len;
};


void nrf_rcv_pkt_end(void)
{
	CE_LOW();
	nrf_cmd(C_FLUSH_RX);
	nrf_write_reg(R_STATUS,R_STATUS_RX_DR);
};
