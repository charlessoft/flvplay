flvplay
=======

An nginx plugin that play flv movies for UDS.


Nginx插件开发需求
-----------------
 
###描述

业务系统请求Nginx服务器上的服务，地址如`http://IP:Port/Contextpath/flvplay?sysCheckNo=&documentId=&versionId=`， 其中，flvplay是Nginx服务器提供的请求服务。

开发flvplay请求服务，通过该服务解析相应的请求参数，封装请求参数成json串格式

    {"sysCheckNo":"系统验证码","documentId":"文档ID","versionId":"版本ID"}

通过调用非结构化平台提供的Http接口(post方式)，获取该flv在Nginx上的相对路径，
如`/flv/00/0b/01.flv`，由此请求服务组装该flv在Nginx上的实际路径
`http://IP:Port/Contextpath/flv/00/0b/01.flv`，并转向到组装后的请求地址，使得该flv在IE上能够实现播放。
