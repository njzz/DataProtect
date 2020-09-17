#include <jni.h>
#include <string>
#include "helpers.h"
#include "DataChange.h"
#include "DHP.h"
#include "FileOpt.h"

static std::string g_sskey;

extern "C" JNIEXPORT jboolean JNICALL
Java_com_njzz_dataprotect_MainActivity_CreateKey(
        JNIEnv* env,
        jobject /* this */,jstring filesave,jstring key) {
    auto cfile=env->GetStringUTFChars(filesave,0);//转换成const char *
    auto ckey=env->GetStringUTFChars(key,0);//转换成const char *
    g_sskey=DP::CreateAndSave(cfile,ckey);
    jboolean jr=(jboolean)(g_sskey.size()>0?1:0);
    env->ReleaseStringUTFChars(filesave,cfile);
    env->ReleaseStringUTFChars(key,ckey);
    return jr;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_njzz_dataprotect_MainActivity_TestKey(
        JNIEnv* env,
        jobject /* this */,jstring filesave,jstring key) {
    auto cfile=env->GetStringUTFChars(filesave,0);//转换成const char *
    auto ckey=env->GetStringUTFChars(key,0);//转换成const char *
    g_sskey=DP::AuthInFile(cfile,ckey);
    jboolean jr=(jboolean)(g_sskey.size()>0?1:0);
    env->ReleaseStringUTFChars(filesave,cfile);
    env->ReleaseStringUTFChars(key,ckey);
    return jr;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_njzz_dataprotect_MainActivity_IsKeyFileValid(
        JNIEnv* env,
        jobject /* this */,jstring filetest) {
    auto cfile=env->GetStringUTFChars(filetest,0);//转换成const char *
    jboolean jr=(jboolean)(DP::IsKeyFileValid(cfile)?1:0);
    env->ReleaseStringUTFChars(filetest,cfile);
    return jr;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_njzz_dataprotect_MainActivity_FileOpt(
        JNIEnv* env,
        jobject /* this */,jstring filein,jstring fileout,jboolean bEncode) {

    auto sfilein=env->GetStringUTFChars(filein,0);//转换成const char *
    auto sfileout=env->GetStringUTFChars(fileout,0);//转换成const char *
    jboolean rt= (jboolean)(DP::ProcessFile(sfilein,sfileout,g_sskey,bEncode!=0)?1:0);
    env->ReleaseStringUTFChars(filein,sfilein);
    env->ReleaseStringUTFChars(fileout,sfileout);
    return rt;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_njzz_dataprotect_MainActivity_EncodeStr2File(
        JNIEnv* env,
        jobject /* this */,jstring strin,jstring fileout) {

    auto cinfo=env->GetStringUTFChars(strin,0);//转换成const char *
    auto cfileout=env->GetStringUTFChars(fileout,0);//转换成const char *
    jboolean rt= (jboolean)(DP::StrEncode2File(cinfo,strlen(cinfo),cfileout,g_sskey)?1:0);
    env->ReleaseStringUTFChars(strin,cinfo);
    env->ReleaseStringUTFChars(fileout,cfileout);
    return rt;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_njzz_dataprotect_MainActivity_DecodeFile2Str(
        JNIEnv* env,
        jobject /* this */,jstring filein) {

    auto cfilein=env->GetStringUTFChars(filein,0);//转换成const char *
    std::string out;
    DP::FileDecode2Str(cfilein,out,g_sskey);
    env->ReleaseStringUTFChars(filein,cfilein);
    return env->NewStringUTF(out.c_str());
}