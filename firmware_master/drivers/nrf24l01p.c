#include <drivers/nrf24l01p.h>
#include <main.h>

#define DEFAULT_SPEED R_RF_SETUP_DR_2M

uint8_t _nrfresets=0;

/*-----------------------------------------------------------------------*/
/* Transmit a byte via SPI                                               */
/*-----------------------------------------------------------------------*/
inline void xmit_spi(uint8_t dat) {
    ssp1_send_byte (dat);
//    sspSend(0, (uint8_t*) &dat, 1);
}


/*#define CS_LOW()    gpioSetValue(RB_SPI_NRF_CS, 0)
#define CS_HIGH()   gpioSetValue(RB_SPI_NRF_CS, 1)
#define CE_LOW()    gpioSetValue(RB_NRF_CE, 0)
#define CE_HIGH()   gpioSetValue(RB_NRF_CE, 1)
*/

#define CE_HIGH()    (LPC_GPIO2->FIOPIN |=  (1 << 6))
#define CE_LOW()    (LPC_GPIO2->FIOPIN &= ~(1 << 6))
#define CS_HIGH()    (LPC_GPIO2->FIOPIN |=  (1 << 2))
#define CS_LOW()    (LPC_GPIO2->FIOPIN &= ~(1 << 2))


void nrf_cmd(uint8_t cmd){
    CS_LOW();
    xmit_spi(cmd);
    CS_HIGH();
};




void nrf_write_reg(const uint8_t reg, const uint8_t val){
    CS_LOW();
    xmit_spi(C_W_REGISTER | reg);
    xmit_spi(val);
    CS_HIGH();
};



void nrf_write_long(const uint8_t cmd, int len, const uint8_t* data){
    CS_LOW();
    xmit_spi(cmd);
    sspSend(0,data,len);
    CS_HIGH();
};

#define nrf_write_reg_long(reg, len, data) \
    nrf_write_long(C_W_REGISTER|(reg), len, data)




void nrf_set_rx_mac(int pipe, int rxlen, int maclen, const uint8_t * mac){
#ifdef SAFE
    assert(maclen>=1 || maclen<=5);
    assert(rxlen>=1 || rxlen<=32);
    assert(pipe>=0 || pipe<=5);
    assert(mac!=NULL);
    if(pipe>1)
        assert(maclen==1);
#endif
    nrf_write_reg(R_RX_PW_P0+pipe,rxlen);

    nrf_write_reg_long(R_RX_ADDR_P0+pipe,maclen,mac);
//    nrf_write_reg(R_EN_RXADDR, 
//            nrf_read_reg(R_EN_RXADDR) | (1<<pipe)
//            );
};

void nrf_set_tx_mac(int maclen, const uint8_t * mac){
#ifdef SAFE
    assert(maclen>=1 || maclen<=5);
    assert(mac!=NULL);
#endif
    nrf_write_reg_long(R_TX_ADDR,maclen,mac);
};


void nrf_set_channel(int channel){
#ifdef SAFE
    assert(channel &~R_RF_CH_BITS ==0);
#endif
    nrf_write_reg(R_RF_CH, channel);
};

void nrf_config_set(nrfconfig config){
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



void nrf_init() {
    // Enable SPI correctly
//    sspInit(0, sspClockPolarity_Low, sspClockPhase_RisingEdge);




    // Enable CS & CE pins
//    gpioSetDir(RB_SPI_NRF_CS, gpioDirection_Output);
//    gpioSetPullup(&RB_SPI_NRF_CS_IO, gpioPullupMode_Inactive);
//    gpioSetDir(RB_NRF_CE, gpioDirection_Output);
//    gpioSetPullup(&RB_NRF_CE_IO, gpioPullupMode_PullUp);
    CE_LOW();

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
    xmit_spi(C_W_TX_PAYLOAD);
    sspSend(0,pkt,size);
    CS_HIGH();

    CE_HIGH();
    delay_ms(1); // Send it.  (only needs >10ys, i think)
    CE_LOW();

	return 0;
//    return nrf_cmd_status(C_NOP);
};


