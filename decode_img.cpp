#include "decode_img.h"
#include <QObject>
#include <QQuickImageProvider>
#include <QPixmap>
#include <QByteArray>
#include "api.h"
#include <QDebug>
#include <QThread>
#include <QFile>
extern "C"{
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libswscale/swscale.h>
    #include <libavutil/pixfmt.h>
    #include<libavutil/time.h>
    #include <libswresample/swresample.h>
}

ImageSrc::ImageSrc():QQuickImageProvider(QQuickImageProvider::Image){};
QImage ImageSrc::requestImage(const QString &id,QSize *size,const QSize &requestedSize){
//    qDebug()<<"image update";
    return image_src;
}
//QPixmap ImageSrc::requestPixmap(const QString &id,QSize *size,const QSize &requestedSize){
//    qDebug()<<"chufa";
//    QPixmap tmp_pix_map;
//    tmp_pix_map.loadFromData(image_byte_array);
//    return  tmp_pix_map;
//}


av_packet_open::av_packet_open(QObject *parent):QObject (parent){

}
void av_packet_open::start(){
        qDebug()<<"viduo 开始";
        QString file_path="/home/wangqinfeng/下载/a.mp4";
        QFile meide_file(file_path);
        if(!meide_file.exists()){
            qDebug()<<"文件不存在";
            return;
        }
        av_register_all();
        const char *file_path_const_char=file_path.toLocal8Bit().data();
        AVFormatContext *pFormatCtx; //AV格式化上下文
        AVCodecContext *vidoCodecCtx;  //视频解码器上下文
        AVCodec *vidopCodec;            //视频解码器
        AVFrame *pFrame,*pFrameRGB; //解码后的帧(Audio Video帧)，pFrame实时帧，pFrameRGB帧缓存.
        AVPacket *packet;           //解码前的packet包
        uint8_t *out_buffer;        //输出缓存指针
        AVStream *stream;
        static struct SwsContext *img_convert_ctx; //像素结构上下文
        int videoStream,i,numBytes;
        int ret,got_picture;
        av_register_all(); //注册库
        pFormatCtx=avformat_alloc_context(); //分配一个AVFormatContext
        //打开文件
        if(avformat_open_input(&pFormatCtx,file_path_const_char,NULL,NULL)!=0){ //打开视频文件并绑定AV格式化上下文，返回int 0表示成功
            qDebug()<<"Open File error";
            return;
        }
        if(avformat_find_stream_info(pFormatCtx,NULL)<0){ // 查寻媒体流信息,媒体流数大于0
            qDebug()<<"媒体流小于0";
            return;
        }
        videoStream=-1; //初始化流数-1
        for(i=0;i<pFormatCtx->nb_streams;i++){ //媒体流数循环类型判断
            if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){ //判断媒体流类型是否为视频
                videoStream=i; //赋值媒体流number
                stream=pFormatCtx->streams[i];
            }
        }
        if(videoStream==-1){ //如果没有流程序退出
            qDebug()<<"视频不包含视频流";
            return ;
        }
        //查找解码器
        vidoCodecCtx=pFormatCtx->streams[videoStream]->codec; // 返回解码视频解码器上下文.
        vidopCodec=avcodec_find_decoder(vidoCodecCtx->codec_id);//通过视频解码器ID查指定的解码器，并实例化解码器
        if(vidopCodec==NULL){
            qDebug()<<"此视频的解码器不存在";
            return;
        }
        //打开解码器
        if(avcodec_open2(vidoCodecCtx,vidopCodec,NULL)<0){ //小于0表示打开不成功
            qDebug()<<"视频解码器打开失败";
            return ;
        }
    //    avcodec_open(audioCodecCtx,audioCodec);
        pFrame=av_frame_alloc();//初始化AVFrame帧结构体
        pFrameRGB=av_frame_alloc(); //初始化AVFrame结构体
        img_convert_ctx=sws_getContext( //返回SwsContext图像处理上下文
                                       vidoCodecCtx->width, //源文件解码器中的文件宽度
                                       vidoCodecCtx->height, //源文件解码器中的文件高度
                                       vidoCodecCtx->pix_fmt, //源文件像素格式
                                       vidoCodecCtx->width,    //输出宽度
                                       vidoCodecCtx->height,   //输出高度
                                       AV_PIX_FMT_RGB32,     //输出像素格式
                                       SWS_BICUBIC,         //输出算法
                                       NULL,NULL,NULL);
        numBytes=avpicture_get_size(AV_PIX_FMT_RGBA,vidoCodecCtx->width,vidoCodecCtx->height); //算出图像大小并返回
    //    qDebug()<<"图像大小"<<numBytes;
        out_buffer=(uint8_t*)malloc(numBytes*sizeof(uint8_t)); //av_malloc/malloc分配一个以字节为的缓存块,图像大小*8bit.
        int y_size=vidoCodecCtx->width*vidoCodecCtx->height;
        //向系统申请分配指定size个字节的内存空间。
        packet=(AVPacket*)malloc(sizeof (AVPacket));
        av_new_packet(packet,y_size); //为pkt分配一个指定大小的内存
    //    av_dump_format(pFormatCtx,0,"/home/wangqinfeng/下载/a.mp4",0);//输出视频信息
        int index=0;
        //为已经分配的空间的结构体AVPicture挂上一段用于保存数据的空间,会把图像数据写入out_buffer,
        avpicture_fill((AVPicture *)pFrameRGB,out_buffer,AV_PIX_FMT_RGB32,vidoCodecCtx->width,vidoCodecCtx->height);
        while (av_read_frame(pFormatCtx,packet)>=0) {
            if(packet->stream_index==videoStream){ //判断是否为视频流
    //            continue;
                ret=avcodec_decode_video2(vidoCodecCtx,pFrame,&got_picture,packet); /*对编码包进行解码,(解码器上下文,av帧结构体,int 执行结果状态,编码包),对packet编码包
                采用pCodecCtx解码器进行解码,并把解码后的帧存入pFrame,并把结果状态存入got_picture;*/
                if(ret<0){ //"解码失败"
                    qDebug()<<"decode error";
                    return;
                }
                if(got_picture){//"编码包解帧成功"
                    //YUV帧转rgb帧
                    sws_scale(img_convert_ctx,(uint8_t const * const *)pFrame->data,
                              pFrame->linesize,0,vidoCodecCtx->height,
                              pFrameRGB->data,pFrameRGB->linesize);
                    QImage tmpImg((uchar*)out_buffer,vidoCodecCtx->width,vidoCodecCtx->height,QImage::Format_RGB32);
                    image_src=tmpImg;
                    float vidio_time=av_frame_get_best_effort_timestamp(pFrame)*av_q2d(stream->time_base);
                    float time_base=av_q2d(stream->time_base);
                    float add_time_sleep=sleep_time-vidio_time;
                    if(sleep_time!=0 and (vidio_time>sleep_time)){
                        add_time_sleep=vidio_time-sleep_time;
                        QThread::msleep(25+add_time_sleep+av_q2d(stream->time_base));
                    }else if(sleep_time!=0 and vidio_time<sleep_time) {
                            add_time_sleep=sleep_time-vidio_time;
                              QThread::msleep(25-add_time_sleep+av_q2d(stream->time_base));
                     }
                    add_time_sleep=0;
//                    qDebug()<<vidio_time<<sleep_time;
                    update_image();
                }
        }
            //删除
            av_free_packet(packet);
        }
        //删除
        av_free(out_buffer);
        av_free(pFrameRGB);
        avcodec_close(vidoCodecCtx);
        avformat_close_input(&pFormatCtx);

}
