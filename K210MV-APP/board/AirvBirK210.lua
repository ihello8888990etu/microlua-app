--这是widora AIRVBITK210 开发板配置文件 
board={dev="AirvBirK210",firmware="1.2.0"}
camera={
    RST_PIN = 42,VSYNC_PIN = 43,PWDN_PIN = 44,
	HREF_PIN = 45,PCLK_PIN = 47,XCLK_PIN = 46,
	SCCB_SCLK_PIN = 41,SCCB_SDA_PIN = 40}
	
lcd={RST_PIN=37,DC_PIN=38,CS_PIN=36,SCLK_PIN=39,dev="st7788"}

--[[sdcard={
    SCLK_PIN = 27,D0_PIN = 28,
	D1_PIN = 26,	CS_PIN = 29}]]
	
flash={dev="w25qxx"}