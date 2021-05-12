12345--这是microlua ai 官方开发板的配置文件
board={dev="MaixGo",firmware="1.2.0"}

lcd={RST_PIN=37,DC_PIN=38,CS_PIN=36,SCLK_PIN=39,dev="st7788"}

camera={
    RST_PIN = 42,VSYNC_PIN = 43,PWDN_PIN = 44,
	HREF_PIN = 45,PCLK_PIN = 47,XCLK_PIN = 46,
	SCCB_SCLK_PIN = 41,SCCB_SDA_PIN = 40}
	

flash={dev="w25qxx"}

--[[sdcard={
    SCLK_PIN = 0,D0_PIN = 0,
	D1_PIN = 0,	CS_PIN = 0}]]
	
	