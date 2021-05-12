--这是microlua ai 官方开发板的配置文件
board={dev="OpenAI",firmware="1.2.0",debug=false}

lcd={RST_PIN=45,DC_PIN=46,CS_PIN=44,SCLK_PIN=47,dev="st7788"}

camera={
    RST_PIN = 41,VSYNC_PIN = 40,PWDN_PIN = 39,
	HREF_PIN = 38,PCLK_PIN = 36,XCLK_PIN = 37,
	SCCB_SCLK_PIN = 42,SCCB_SDA_PIN = 43}
	

flash={dev="w25qxx"}

--[[sdcard={
    SCLK_PIN = 0,D0_PIN = 0,
	D1_PIN = 0,	CS_PIN = 0}]]