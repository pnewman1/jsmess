/* Scorpion 4 + 5 driver releated includes */
/* mainly used for stuff which is currently shared between sc4 / 5 sets to avoid duplication */

#define sc_dnd_others \
	ROM_REGION( 0x400000, "ymz", ROMREGION_ERASE00 ) \
	/* Regular */ \
	ROM_LOAD( "95008606.bin", 0x0000, 0x100000, CRC(c63c8663) SHA1(4718baf87340fe93ccfe642a1a0cdb9d9dcac57f) ) /* 0 (1907)  DOND SOUNDS 11 */ \
	ROM_LOAD( "95008607.bin", 0x0000, 0x100000, CRC(a615514f) SHA1(d4ff7d4fe6f1dd1d7b00cc504f90b2921aa5e8fb) ) /* 1 */ \
	\
	/*  Casino */ \
	ROM_LOAD( "95008631.bin", 0x0000, 0x100000, CRC(7208854a) SHA1(a02de60cfcbafe5da4f67596ab65237f5b5f41b7) ) /* 0 (1954) DOND SOUNDS 11 */ \
	\
	ROM_LOAD( "95008632.bin", 0x0000, 0x100000, CRC(fd81a317) SHA1(1e597fd58aab5a7a8321dc4daf138ee07b42c094) ) /* 0 (1945) DOND SOUNDS 11 */ \
	ROM_LOAD( "95008633.bin", 0x0000, 0x100000, CRC(a7a445d4) SHA1(dbb1938c33ce654c2d4aa3b6af8c210f5aad2ae3) ) /* 1 */ \
	\
	/*  Casino */ \
	ROM_LOAD( "95008661.bin", 0x0000, 0x100000, CRC(2d9ebcd5) SHA1(d824a227420cbe616aca6e2fd279af691ddfd87a) ) /* 0 (1945) DOND SOUNDS 12 */ \
	\
	ROM_LOAD( "95008680.bin", 0x0000, 0x100000, CRC(9bd439d1) SHA1(5e71d04e5697e92998bae28f7352ea7742cafe07) ) /* 0 (1964) DOND SOUNDS 11 */ \
	\
	ROM_LOAD( "95008698.bin", 0x0000, 0x100000, CRC(8eea7754) SHA1(7612c128d6c062bba3477d55aee3089e1255f61e) ) /* 0 (1964) DOND SOUNDS 12 */ \


#define sc_dndbb_others \
	ROM_REGION( 0x400000, "ymz", ROMREGION_ERASE00 ) \
	ROM_LOAD( "95008788.bin", 0x0000, 0x100000, CRC(51384d64) SHA1(3af87bcc7fb4881ae671d9cbb484fb6a2a534cce) ) \
	ROM_LOAD( "95008789.bin", 0x0000, 0x100000, CRC(4f2d72c3) SHA1(f167ad8d7178682833e7e9efcc393b5826108d70) ) \


#define sc_dndcl_others \
	ROM_REGION( 0x400000, "ymz", ROMREGION_ERASE00 ) \
	/* club dond */ \
	ROM_LOAD( "95008629.bin", 0x0000, 0x100000, CRC(6f7cf500) SHA1(bf3db728872251116b32aa2c44f72a87d29b13d2) ) \
	ROM_LOAD( "95008630.bin", 0x0000, 0x100000, CRC(5dfda41e) SHA1(6d61d0eedc6a1599777f47d3880689f954f326c9) ) \
	/* beat the banker */ \
	ROM_LOAD( "95008713.bin", 0x0000, 0x100000, CRC(7311db5e) SHA1(24a4b3f44fa6dbbe756b0acac2b513c6e92cdf26) ) /* R3068? */ \
	ROM_LOAD( "95008714.bin", 0x0000, 0x100000, CRC(c6ebc845) SHA1(6a135f42940b8c44422736650e369fa12b51e0df) ) \

#define sc_dnddd_others \
	ROM_REGION( 0x400000, "ymz", ROMREGION_ERASE00 ) \
	/* dond double */ \
	ROM_LOAD( "95008804.bin", 0x0000, 0x100000, CRC(19fd19fa) SHA1(db68a40d8bb3de130c6d5777217fb3c3a4c13d17) ) \
	ROM_LOAD( "95008805.bin", 0x0000, 0x100000, CRC(f497337d) SHA1(9e827116f8b1d882cfcd0b56aaede9de14b2e943) ) \

#define sc_dndhf_others \
	ROM_REGION( 0x400000, "ymz", ROMREGION_ERASE00 ) \
	ROM_LOAD( "95008705.bin", 0x0000, 0x100000, CRC(09e02843) SHA1(a6ac658c857eca2aca0bacda423dd012434e93bc) ) \
	ROM_LOAD( "95008706.bin", 0x0000, 0x100000, CRC(805e7eb8) SHA1(107c6ba26e37d8f2eec81b62c3edf3efb7e44ca2) ) \

#define sc_dndys_others \
	ROM_REGION( 0x400000, "ymz", ROMREGION_ERASE00 ) \
	ROM_LOAD( "95008779.bin", 0x0000, 0x100000, CRC(cf04d115) SHA1(3255b58cd4fba7d231ca35c00d1cb70da1f6cfbd) ) \
	ROM_LOAD( "95008780.bin", 0x0000, 0x100000, CRC(ec270dbe) SHA1(f649ffd4530feed491dc050f40aa0205f4bfdd89) ) \

#define sc_dndww_others \
	ROM_REGION( 0x400000, "ymz", ROMREGION_ERASE00 )  \
	ROM_LOAD( "95008730.bin", 0x0000, 0x100000, CRC(e2aebdb0) SHA1(d22fee7ff3d5912ea9a7440ec82de52a7d016090) ) /* casino wow */ \

#define sc_dndtr_others \
	ROM_REGION( 0x400000, "ymz", ROMREGION_ERASE00 ) \
	ROM_LOAD( "95008832.bin", 0x0000, 0x100000, CRC(1d2aa204) SHA1(52ad7a6ea369198b48d593de508f317ed11e84de) ) \
	ROM_LOAD( "95008833.bin", 0x0000, 0x100000, CRC(92f462f6) SHA1(c0782639d4cd2653a3d14e1b422c2d6117dac029) ) \

#define sc_dndwb_others \
	ROM_REGION( 0x400000, "ymz", ROMREGION_ERASE00 ) \
	ROM_LOAD( "95008651.bin", 0x0000, 0x100000, CRC(c3738d54) SHA1(bf3c62e196a2cea144868729f136000a7b924c3d) ) /* wiyb */ \
	ROM_LOAD( "95008652.bin", 0x0000, 0x100000, CRC(ba8f5b62) SHA1(2683623e8b90d5e9586295d720aa1d985b416117) )  \
	ROM_LOAD( "95008715.bin", 0x0000, 0x100000, CRC(1796b604) SHA1(9045f9424e6447d696a6fdd7f5bdcbfda4b57c90) ) /* wiyb */ \
	ROM_LOAD( "95008766.bin", 0x0000, 0x100000, CRC(62accb81) SHA1(ed0456cefed2fbf9a1cfd911f871262e892a4ce8) ) /* wiyb */ \
	ROM_LOAD( "95008767.bin", 0x0000, 0x100000, CRC(0aa49d8a) SHA1(a2baffc495656f0b5426b720cac298a78774b7fa) ) /* super slam?? */  \

#define sc_dndpg_others \
	ROM_REGION( 0x400000, "ymz", ROMREGION_ERASE00 ) \
	ROM_LOAD( "95008822.bin", 0x0000, 0x100000, CRC(16411f05) SHA1(11f2d17b6647b9d9e713e38bab6604e4feb51432) ) \
	ROM_LOAD( "95008823.bin", 0x0000, 0x100000, CRC(4f7bab89) SHA1(bbb203cff3212571c313e8f22f9083980baf2566) ) \

#define sc_dndtp_others \
	ROM_REGION( 0x400000, "ymz", ROMREGION_ERASE00 ) \
	ROM_LOAD( "95008673.bin", 0x0000, 0x100000, CRC(4b8d1e0b) SHA1(8cba9632abb2800f8e9a45d1bf0abbc9abe8cb8c) ) \
	ROM_LOAD( "95008674.bin", 0x0000, 0x100000, CRC(66236e20) SHA1(222fefdeddb22eb290302528a8f937468ccd5698) ) \
	

#define sc_dndbe_others \
	ROM_REGION( 0x400000, "ymz", ROMREGION_ERASE00 ) \
	ROM_LOAD( "95008624.bin", 0x0000, 0x100000, CRC(bf9620ea) SHA1(63f5a209da3d0117fcb579364a53b23d2b02cfe5) ) \
	ROM_LOAD( "95008625.bin", 0x0000, 0x100000, CRC(2e1a1db0) SHA1(41ebad0615d0ad3fea6f2c00e2bb170d5e417e4a) ) \

#define sc_dndbc_others \
	ROM_REGION( 0x400000, "ymz", ROMREGION_ERASE00 ) \
	ROM_LOAD( "95009100.bin", 0x0000, 0x100000, CRC(b06110c0) SHA1(84289721a8c71124cc4df79fc90d0ded8d43fd07) ) \
	ROM_LOAD( "95009101.bin", 0x0000, 0x100000, CRC(53b38d0a) SHA1(1da40cbee8a18713864e3a578ac49c2108585e44) ) \

#define sc_dndlp_others \
	ROM_REGION( 0x400000, "ymz", ROMREGION_ERASE00 ) \
	ROM_LOAD( "95008792.bin", 0x0000, 0x100000, CRC(2e7e1a5a) SHA1(000cb48c67b7f23d00318d37206c2df426c79281) ) \
	ROM_LOAD( "95008793.bin", 0x0000, 0x100000, CRC(4b91b638) SHA1(b97cb1d22f91a791fc4d47bbae8965882898a317) ) \

#define sc_dndra_others \
	ROM_REGION( 0x400000, "ymz", ROMREGION_ERASE00 ) \
	ROM_LOAD( "95008752.bin", 0x0000, 0x100000, CRC(82bfd510) SHA1(857f294d46d64275b15c56187bbbc19e2aa0f5bc) ) \
	ROM_LOAD( "95008753.bin", 0x0000, 0x100000, CRC(ee0b5da9) SHA1(f6cb6037f525df504c1ba8106f19990ecf8d1bd2) ) \

#define sc_dndbd_others \
	ROM_REGION( 0x400000, "ymz", ROMREGION_ERASE00 ) \
	ROM_LOAD( "95008790.bin", 0x0000, 0x100000, CRC(3c56a8b6) SHA1(8c06aa725233f4feb7c2e703f203cf9b0c6669a1) ) \
	ROM_LOAD( "95008791.bin", 0x0000, 0x100000, CRC(7378a4a8) SHA1(af9dd1fcbd8b77fab4afc85a325c0eb011f35e3a) ) \

#define sc_dndbr_others \
	ROM_REGION( 0x400000, "ymz", ROMREGION_ERASE00 ) \
	ROM_LOAD( "95008723.bin", 0x0000, 0x100000, CRC(6e1d9bd5) SHA1(92b2fd0b75a195af6ddfbdee3316f8af4bc4eb1a) ) \
	ROM_LOAD( "95008724.bin", 0x0000, 0x100000, CRC(309c1bf1) SHA1(82811b503a06e30a915eebdbdbcd63b567f241c1) ) \

#define sc_dndcc_others \
	ROM_REGION( 0x400000, "ymz", ROMREGION_ERASE00 ) \
	ROM_LOAD( "95008746.bin", 0x0000, 0x100000, CRC(ef3c39b9) SHA1(21b9788080de68acd436608d7d46fd9663d1589b) ) /* crazy chair */ \
	ROM_LOAD( "95008747.bin", 0x0000, 0x100000, CRC(9f9b638f) SHA1(5eae49f8a9571f8fade9acf0fc80ea3d70cc9e18) ) \
	\
	ROM_LOAD( "95008840.bin", 0x0000, 0x100000, CRC(2aa17d67) SHA1(43bdee1b3d0a0bf28f672620867fa3dc95727fbc) ) /* crazy chair */ \
	ROM_LOAD( "95008841.bin", 0x0000, 0x100000, CRC(594a9cd7) SHA1(667fa3106327ce4ff23877f2d48d3e3e360848d0) ) \
	\
	ROM_LOAD( "95009131.bin", 0x0000, 0x100000, CRC(59a9e50a) SHA1(15d79b177a1e926cd5aee0f969f5ef8b30fac203) ) /* crazy chair */ \
	ROM_LOAD( "95009132.bin", 0x0000, 0x100000, CRC(5e8a7ca6) SHA1(419ecc6ac15004bdd83bcd5b3e00d9dcd0d24936) ) \

#define sc_dnddw_others \
	ROM_REGION( 0x400000, "ymz", ROMREGION_ERASE00 ) \
	ROM_LOAD( "95008754.bin", 0x0000, 0x100000, CRC(d2c3e3e5) SHA1(3d21c812456618471a331d596760ea8746afc036) ) \
	ROM_LOAD( "95008755.bin", 0x0000, 0x100000, CRC(e6906180) SHA1(81215fd3dfe315123d5b028047a93e30baa52b5d) ) \

#define sc_dnddf_others \
	ROM_REGION( 0x400000, "ymz", ROMREGION_ERASE00 ) \
	ROM_LOAD( "95008738.bin", 0x0000, 0x100000, CRC(2a06f488) SHA1(666bd9c8091edda4b003fa586e5ae270831c119f) ) \
	ROM_LOAD( "95008739.bin", 0x0000, 0x100000, CRC(33d677b0) SHA1(5283d23671e340e3fda4eb4c6795d1d56b073206) ) \

#define sc_mowow_others \
	ROM_REGION( 0x5000, "pics", 0 ) \
	ROM_LOAD( "95890712.bin", 0x0000, 0x5000, CRC(ec6db00b) SHA1(d16a1527caa3c115e3326c897ce0fa66e3a0420d) ) \
	ROM_LOAD( "95890713.bin", 0x0000, 0x5000, CRC(f0bb40b7) SHA1(33c19dab3086cdeae4f503fbf3f3cc5f0dad98c4) ) \
	ROM_LOAD( "95890714.bin", 0x0000, 0x5000, CRC(33e16227) SHA1(87efc1a046ef6af0b72cc76a6ee393a4d1ddbce3) ) \
	ROM_REGION( 0x400000, "ymz", ROMREGION_ERASE00 ) \
	ROM_LOAD( "95008550.bin", 0x0000, 0x100000, CRC(db6343bf) SHA1(e4d702020af67aa5be0560027706c1cbf34296fa) ) \
	ROM_LOAD( "95008551.bin", 0x0000, 0x100000, CRC(2d89a52a) SHA1(244101df7f6beae545f9b823750f908f532ac1e4) ) \
	ROM_LOAD( "95008850.bin", 0x0000, 0x0af41f, CRC(8ca16e09) SHA1(9b494ad6946c2c7bbfad6591e62fa699fd53b6dc) ) \
	ROM_LOAD( "95008869.bin", 0x0000, 0x0b9d9d, CRC(f3ef3bbb) SHA1(92f9835e96c4fc444a451e97b2b8a7b66e5794b7) ) \

#define sc_nunsm_others \
	ROM_REGION( 0x400000, "ymz", ROMREGION_ERASE00 ) \
	ROM_LOAD( "95008522.bin", 0x0000, 0x0f9907, CRC(df612d06) SHA1(cbca56230c4ad4c6411aa5c2e2ca2ae8152b5297) ) \


#define sc_gfev_others \
	ROM_REGION( 0x400000, "ymz", ROMREGION_ERASE00 ) \
	ROM_LOAD( "95416146.bin", 0x0000, 0x100000, CRC(17e92fa0) SHA1(7dea5166f3f70e5d249da56f01bbe2267ce43d6a) ) \


