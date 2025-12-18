版本注意：
1、加载KO命令需要带sensor名：./load3516cv610_debug -a -profile 1 -sensor0 sc500ai
2、64M内存场景只支持起xx_10b.ini（610）、xx_608.ini（608）业务；只支持线性模式，性能限制不支持抓灌数据，特殊分辨率业务点播需减小configs\common目录下config_mt.ini中bufcoef值，建议改为200；
   如果需要支持抓灌，应降低分辨率、关闭vi和vpss卷绕、增大VB、设置全离线模式；可参考以下说明设置ini：
   ①分辨率降为2304*1024（宽高取值需为4的倍数）
   ②[vi_pipe.0]wrap_en置为FALSE，[vpss_chn_wrap.0.0]wrap_en置为FALSE
   ③[vbblk.0]删除，[vbblk.1]改为[vbblk.0]，blk_cnt增大到4
   ④vi_vpss_mode改为0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0
3、单板性能限制，500ai wdr业务 支持灌raw但抓数据报错
4、单板性能限制，部分业务场景不支持抓数据，需修改vi_vpss_mode= 1|1|1|1|0|0|0|0|0|0|0|0|0|0|0|0为0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0，但是修改后logmpp会刷屏报错vb不足
5、起VI VPSS全在线模式：修改ViVpssMode = 3|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0
6、os04d10默认ini业务只支持跑lane_id为0|1的sensor；跑lane_id为0|2的sensor，需要修改mipi_attr.lane_id = 0|1|-1|-1|-1|-1|-1|-1|为0|2|-1|-1|-1|-1|-1|-1|