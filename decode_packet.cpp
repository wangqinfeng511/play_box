#include "decode_packet.h"
#include <QFile>
#include <QDir>
#include <QImage>
#include <QString>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QThread>
#include <QDebug>
#include "image_src.h"
extern "C"{
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libswscale/swscale.h>
    #include <libavutil/pixfmt.h>
    #include <libswresample/swresample.h>
}
decode_packet::decode_packet(QObject *parent) : QObject(parent)
{

}
void decode_packet::decode_master(QString file_path){
    QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(2);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);
    QAudioOutput *audio=new QAudioOutput(format,0);
    QIODevice *io =audio->start();

    QFile meide_file(file_path);
    if(!meide_file.exists()){
        qDebug()<<"文件不存在";
        return;
    }
    av_register_all();
    const char *file_path_const_char=file_path.toLocal8Bit().data();
    AVFormatContext *pFormatCtx; //AV格式化上下文
    AVCodecContext *vidoCodecCtx,*audioCodecCtx;  //视频解码器上下文
    AVCodec *vidopCodec,*audioCodec;            //视频解码器
    AVFrame *pFrame,*pFrameRGB,*audoframe; //解码后的帧(Audio Video帧)，pFrame实时帧，pFrameRGB帧缓存.
    AVPacket *packet;           //解码前的packet包
    uint8_t *out_buffer;        //输出缓存指针
    static struct SwsContext *img_convert_ctx; //像素结构上下文
    int videoStream,audioStream,i,numBytes;
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
    audioStream=-1;
    for(i=0;i<pFormatCtx->nb_streams;i++){ //媒体流数循环类型判断
        if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){ //判断媒体流类型是否为视频
            videoStream=i; //赋值媒体流number
        }else if (pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO) {
            audioStream=i;
}
    }
    if(videoStream==-1){ //如果没有流程序退出
        qDebug()<<"视频不包含视频流";
        return ;
    }
    //查找解码器
    if(audioStream==-1){
        qDebug()<<"无音频流";
    }
    vidoCodecCtx=pFormatCtx->streams[videoStream]->codec; // 返回解码视频解码器上下文.
    audioCodecCtx=pFormatCtx->streams[audioStream]->codec;
    qDebug()<<audioCodecCtx->codec_type;
    vidopCodec=avcodec_find_decoder(vidoCodecCtx->codec_id);//通过视频解码器ID查指定的解码器，并实例化解码器
    audioCodec=avcodec_find_decoder(audioCodecCtx->codec_id);
    if(vidopCodec==NULL){
        qDebug()<<"此视频的解码器不存在";
        return;
    }
    if(audioCodec==NULL){
        qDebug()<<"此音频的解码器不存在";
        return;
    }
    //打开解码器
    if(avcodec_open2(vidoCodecCtx,vidopCodec,NULL)<0){ //小于0表示打开不成功
        qDebug()<<"视频解码器打开失败";
        return ;
    }
     if(avcodec_open2(audioCodecCtx,audioCodec,NULL)<0){ //小于0表示打开不成功
        qDebug()<<"音频解码器打开失败";
        return ;
    }
//    avcodec_open(audioCodecCtx,audioCodec);
    pFrame=av_frame_alloc();//初始化AVFrame帧结构体
    audoframe=av_frame_alloc();
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
    SwrContext *swrCtr=swr_alloc();
    enum AVSampleFormat in_sample_fmt=audioCodecCtx->sample_fmt;
    enum AVSampleFormat out_sample_fmt=AV_SAMPLE_FMT_S16;
    int in_sample_rate=audioCodecCtx->sample_rate;
    int out_sample_rate=44100;
    uint64_t in_ch_layout=audioCodecCtx->channel_layout;
    uint64_t out_ch_layout=AV_CH_LAYOUT_STEREO;
    SwrContext *swr=swr_alloc_set_opts(swrCtr,out_ch_layout,out_sample_fmt,out_sample_rate,
                               in_ch_layout,in_sample_fmt,in_sample_rate,0,NULL);
    swr_init(swr);
    while (av_read_frame(pFormatCtx,packet)>=0) {
        int len=0;
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
//                image_byte_array=QByteArray((char *) out_buffer);
                update_image();
//                QThread::msleep(20);
            }
    }else if (packet->stream_index==audioStream) {
            index+=1;
            int bianma= audioCodecCtx->sample_rate;
            ret=avcodec_decode_video2(audioCodecCtx,audoframe,&got_picture,packet);
            if(ret>0){
                if(got_picture){
                    int frame_data_size=audoframe->nb_samples;
                        int out_channel_nb=av_get_channel_layout_nb_channels(out_ch_layout);
                        uint8_t *out_buffer=(uint8_t*)malloc(44100*4);
                        len=swr_convert(swrCtr,&out_buffer,44100*4,
                                    (const uint8_t **)audoframe->data,
                                     audoframe->nb_samples
                                );
                    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
                   if(!info.isFormatSupported(format)){
                                         qWarning()<<"Raw audio format not supported by backend, cannot play audio.";
                                         return;
                                     }
                   int size=len * out_channel_nb * av_get_bytes_per_sample(out_sample_fmt);
                    io->write((char*)out_buffer,size);
                    QThread::msleep(20);
                }

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




//    if(avformat_open_input(&pFormatCtx,file_path_const_char,NULL,NULL)!=0){
//        qDebug()<<"打开文件失败，请确定权限";
//        return;
//    }
//    int streame_number=pFormatCtx->nb_streams;
//    if(streame_number<1){
//        qDebug()<<"非视频文件";
//        return;
//    }
//    int vido_streame_number=-1;
//    int audio_streame_number=-1;
//    for(int i=0;i<streame_number;i++){
//        if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){
//            vido_streame_number=i;
//        }else if (pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO) {
//        audio_streame_number=i;
//    }
//   }
//   AVCodecContext *video_CodeCtx;
//   AVCodec *video_code;
//   video_CodeCtx=pFormatCtx->streams[vido_streame_number]->codec;
//   video_code=avcodec_find_decoder(video_CodeCtx->codec_id);
//   if(avcodec_open2(video_CodeCtx,video_code,NULL)<0){
//       qDebug()<<"视频编码器打开失败";
//       return;
//   }
//   AVCodecContext *Aduio_CodeCtx;
//   AVCodec *audio_code;
//   Aduio_CodeCtx=pFormatCtx->streams[audio_streame_number]->codec;
//   audio_code=avcodec_find_decoder(Aduio_CodeCtx->codec_id);
//   if(avcodec_open2(Aduio_CodeCtx,audio_code,NULL)<0){
//       qDebug()<<"音频解码器打开失败";
//   }
//   AVPacket *packet=(AVPacket*)malloc(sizeof (AVPacket));
//    int y_size=video_CodeCtx->width*video_CodeCtx->height;
//    av_new_packet(packet,y_size);
//   //图像上下文结构体声明
//   static struct SwsContext *img_convert_ctx;

//   //通用帧
//   AVFrame *frame=av_frame_alloc();
//   //图像临时帧
//   AVFrame *rgbframe=av_frame_alloc();
//   while (av_read_frame(pFormatCtx,packet)>=0){
//       if(packet->stream_index==vido_streame_number){
//           int go_picture;
//           int ret=avcodec_decode_video2(video_CodeCtx,frame,&go_picture,packet);
//           if(ret<0){
//               qDebug()<<"获取视频帧失败";
//           };
//           if(go_picture){
//               //rgb32图像需要的缓存大小
//               int rgb_imge_size=(AV_PIX_FMT_RGBA,video_CodeCtx->width,video_CodeCtx->height);
//               //yuv转rgb buffer
//               uint8_t *yuv_rgb_out_buffer;
//               yuv_rgb_out_buffer=(uint8_t*)malloc(rgb_imge_size*sizeof (uint8_t));
//               avpicture_fill((AVPicture *)rgbframe,yuv_rgb_out_buffer,AV_PIX_FMT_RGB32,video_CodeCtx->width,video_CodeCtx->height);
//               img_convert_ctx=sws_getContext(
//                                  video_CodeCtx->width,
//                                  video_CodeCtx->height,
//                                  video_CodeCtx->pix_fmt,
//                                  video_CodeCtx->width,
//                                  video_CodeCtx->height,
//                                  AV_PIX_FMT_RGB32,
//                                  SWS_BICUBIC,
//                                  NULL,NULL,NULL);
//               qDebug()<<video_CodeCtx->width<<video_CodeCtx->height<<video_CodeCtx->pix_fmt<<video_CodeCtx->width<<video_CodeCtx->height<<AV_PIX_FMT_RGB32<<SWS_BICUBIC;
//               sws_scale(
//                           img_convert_ctx,
//                           frame->data,
//                           frame->linesize,
//                           0,
//                           video_CodeCtx->height,
//                           rgbframe->data,
//                           rgbframe->linesize
//                           );

////                QImage tmp_Img((uchar *)yuv_rgb_out_buffer,rgbframe->width,rgbframe->height,QImage::Format_RGB32);
////                QDir pwd("./png");
////                if(!pwd.exists()){
////                    pwd.mkpath("./");
////                }
////                QString file_path=QString("%1/").arg(pwd.absolutePath())+QString("a%1.png").arg(1);
//////                qDebug()<<file_path;
////                tmp_Img.save(file_path);
//           }
//       }else if (packet->stream_index==audio_streame_number) {

//        }
//   }
}
