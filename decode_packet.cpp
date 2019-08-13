#include "decode_packet.h"
#include <QFile>
#include <QDir>
#include <QImage>
#include <QString>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QThread>
#include <QDebug>
#include "api.h"
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
    AVCodecContext *audioCodecCtx;  //视频解码器上下文
    AVCodec *audioCodec;            //视频解码器
    AVFrame *audoframe; //解码后的帧(Audio Video帧)，pFrame实时帧，pFrameRGB帧缓存.
    AVPacket *packet;           //解码前的packet包
    uint8_t *out_buffer;        //输出缓存指针
    AVStream *stream;
    int audioStream,i;
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
    audioStream=-1;
    for(i=0;i<pFormatCtx->nb_streams;i++){ //媒体流数循环类型判断
        if (pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO) {
            audioStream=i;
            stream=pFormatCtx->streams[i];
        }
    }
    //查找解码器
    if(audioStream==-1){
        qDebug()<<"无音频流";
    }
    audioCodecCtx=pFormatCtx->streams[audioStream]->codec;
    audioCodec=avcodec_find_decoder(audioCodecCtx->codec_id);
    if(audioCodec==NULL){
        qDebug()<<"此音频的解码器不存在";
        return;
    }
    //打开解码器
     if(avcodec_open2(audioCodecCtx,audioCodec,NULL)<0){ //小于0表示打开不成功
        qDebug()<<"音频解码器打开失败";
        return ;
    }
//    avcodec_open(audioCodecCtx,audioCodec);
    audoframe=av_frame_alloc();
    //向系统申请分配指定size个字节的内存空间。
    packet=(AVPacket*)malloc(sizeof (AVPacket));
//    av_dump_format(pFormatCtx,0,"/home/wangqinfeng/下载/a.mp4",0);//输出视频信息
    int index=0;
    //为已经分配的空间的结构体AVPicture挂上一段用于保存数据的空间,会把图像数据写入out_buffer,
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
         if (packet->stream_index==audioStream) {
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
//                   qDebug()<<"音频"<<av_q2d(audioCodecCtx->time_base)<<audoframe->pts;
//                   qDebug("%f",(double)(size/(44100*2*2)));

                   sleep_time=audoframe->pts*av_q2d(stream->time_base)+0.023219955;
                   QThread::msleep(0.023219955*1000);

                }
            }
        }
        //删除
        av_free_packet(packet);

    }
    //删除
    av_free(out_buffer);
    avformat_close_input(&pFormatCtx);
}
