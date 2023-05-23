# CForChatGPT
用c++调用chatGPT接口的方法。use c++ to call chatGPT api.

# 事先准备(precondition)
libcurl.lib,cJSON,一个能翻墙的软件(clash)。
libcurl.lib在windows端需要用vcpkg下载，vcpkg需要用git bash下载。
[vcpkg下载教程](https://www.cnblogs.com/linuxAndMcu/p/14696542.html)
[libcurl下载官网](https://curl.se/download.html)
vcpkg官方推荐放在C://src文件夹下，所以新建C://src文件夹后在gitbash上打开并且将vcpkg项目克隆进来。总的来说就是一下几句指令。
'''
$ cd c:/src
$ git clone https://github.com/microsoft/vcpkg
$ cd vcpkg
$ ./bootstrap-vcpkg.bat
$ ./vcpkg integrate install
$ ./vcpkg install curl:x64-windows
'''
注意最后一个指令的时候不指定x64会默认下载x32的libcurl。
如果一切正常会在C:\src\vcpkg\packages\curl_x64-windows\lib这个路径找到libcurl.lib。
在visual studio2022里配置libcurl.lib的方法为：

![profile](https://github.com/wisefoolish/CForChatGPT/assets/89657017/637ba4cb-0806-447f-9f80-4c6e0a54be73)

![IncludeAndLib](https://github.com/wisefoolish/CForChatGPT/assets/89657017/8c50c8b0-45ea-4f73-b83b-f9d6a894dfb4)

![link](https://github.com/wisefoolish/CForChatGPT/assets/89657017/e8a4b00b-6ecf-4a2c-9f87-991f2179ecfb)

![wherethey](https://github.com/wisefoolish/CForChatGPT/assets/89657017/fac5c17e-34ab-4997-9f88-ffb96fa45baf)

这样就配置好curl环境了。

[cJSON下载地址](https://sourceforge.net/projects/cjson/)

由于chatGPT接口实际上是一个https POST请求，内容为json格式的数据，所以需要进行json解析和json编码。cJSON库帮我们解决了json编解码的问题。
chatGPT api只支持utf-8请求，所以还需要进行编码格式转化。windows中有MultiByteToWideChar和WideCharToMultiByte，Linux有iconv。

最后身在中国的用户需要一个VPN，我用的是clash，能复制clash在cmd上打开的命令先在cmd上翻墙然后运行这段代码连到chatGPT上。

![clashBegin](https://github.com/wisefoolish/CForChatGPT/assets/89657017/d11f2b1b-5418-4e9a-84fa-acd34a308b7a)

![clashNext](https://github.com/wisefoolish/CForChatGPT/assets/89657017/0683b418-1260-4596-a1af-fa08b9bc8125)

![clashFinal](https://github.com/wisefoolish/CForChatGPT/assets/89657017/9e4d41bd-47c3-4c3d-ab10-8c9bf1e8edd1)

# 运行方法(How To Run The Code)
visual studio 打开代码，运行然后打开Release文件夹里编译好的文件，在此文件夹打开cmd，输入clash在cmd里打开的指令(非中国区不需要这一步)，运行release文件夹里编译好的二进制文件。

![翻墙先](https://github.com/wisefoolish/CForChatGPT/assets/89657017/6f307f49-8292-435e-85e0-6f5fe91de58d)

![运行二进制文件](https://github.com/wisefoolish/CForChatGPT/assets/89657017/c55e56fa-c520-4cfd-bffd-1c9a86006258)

# 最后但同样重要(last but not least)
最后但同样是重中之重，别忘了替换chatGPT api 密钥，在$CHATGPT_API_KEY$哪里替换为你自己的密钥，我不能拿我的密钥给你用不是吗？
