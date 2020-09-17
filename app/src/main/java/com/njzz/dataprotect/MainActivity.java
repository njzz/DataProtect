package com.njzz.dataprotect;

import androidx.appcompat.app.AlertDialog;
import androidx.core.app.ActivityCompat;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.ContentUris;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.provider.DocumentsContract;
import android.provider.MediaStore;
import android.view.Gravity;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.Toast;


import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;


public class MainActivity extends Activity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }
    String strData_Path;
    EditText edInfo;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        requestWindowFeature(Window.FEATURE_CUSTOM_TITLE);
        setContentView(R.layout.activity_main);
        getWindow().setFeatureInt(Window.FEATURE_CUSTOM_TITLE, R.layout.title);

        String[] permission={"android.permission.READ_EXTERNAL_STORAGE" ,"android.permission.WRITE_EXTERNAL_STORAGE"};
        ActivityCompat.requestPermissions(this, permission, 1);

        File f= this.getFilesDir();
        final String keyFile = f.getPath()+"/pass.key";
        strData_Path = f.getPath()+"/private.file";
        if(!f.exists() && !f.mkdir()){
            new AlertDialog.Builder(this)
                    .setTitle("提示")
                    .setMessage("错误，不能访问应用目录")
                    .show();
        }else{
            final EditText ed = findViewById(R.id.editText);
            final Button btn = findViewById(R.id.btnGO);
            final boolean bnew = !IsKeyFileValid(keyFile);
            edInfo=findViewById(R.id.editInfo);
            if(!bnew){
                ed.setHint(R.string.hint_auth_tips);
                btn.setText(R.string.btn_auth);
            }

            btn.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    String key = ed.getText().toString();
                    String strTips=null;
                    boolean bSuccessed=false;
                    if(bnew){

                        if(key.length()>3) {
                            if(CreateKey(keyFile,key)) {
                                strTips="请记住你的密码 " + key + " \n不同密码，加密出来的内容是不一样的 \n加密强度不受密码强弱影响";
                                bSuccessed=true;
                            }
                        }else{
                            strTips="密码太短，请至少输入4位";
                        }
                    }else{
                        if(!TestKey(keyFile,key)){
                            strTips="验证失败";
                        }else{
                            bSuccessed=true;
                        }
                    }

                    if(strTips!=null){
                        new AlertDialog.Builder(MainActivity.this,R.style.Theme_AppCompat_Light_Dialog_Alert)
                                .setTitle("提示")
                                .setMessage(strTips)
                                .show();
                    }

                    if(bSuccessed){
                        ShowLoginIn();
                    }
                }
            });


        }

        // Example of a call to a native method

    }

    public void ShowLoginIn(){
        LinearLayout loginLayout = findViewById(R.id.loginLayout);
        loginLayout.setVisibility(View.GONE);

        LinearLayout inLayout = findViewById(R.id.inLayout);
        inLayout.setVisibility(View.VISIBLE);

        //显示标题栏按钮
        Button btn = findViewById(R.id.btnLoad);
        btn.setVisibility(View.VISIBLE);

        btn = findViewById(R.id.btnSaveAs);
        btn.setVisibility(View.VISIBLE);

        btn = findViewById(R.id.btnSave);
        btn.setVisibility(View.VISIBLE);

        //加载现有内容
        String info=DecodeFile2Str(strData_Path);
        edInfo.setText(info);

    }

    public void onLoad(View v) {
        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
        intent.setType("*/*");//无类型限制
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        startActivityForResult( Intent.createChooser(intent, "选择文件导入"), 1);
    }

    public void onSaveAs(View v) {

        byte [] data = read(strData_Path);
        String strPath = Environment.getExternalStorageDirectory().getPath()+"/private.file";
        save(data,strPath);

        Toast tips=Toast.makeText(this,"已另存为根目录 private.file",Toast.LENGTH_SHORT);
        tips.setGravity(Gravity.CENTER,0,0);
        tips.show();
    }

    public byte[] read(String path){
        FileInputStream fin;
        byte[] bytes = null;
        try {
            fin = new FileInputStream(new File(path));
            bytes  = new byte[fin.available()];
            //将文件内容写入字节数组
            fin.read(bytes);
            fin.close();
        } catch (Exception e) {
            e.printStackTrace();
        }

        return bytes;
    }

    public byte[] readUri(Uri uri){
        byte[] buffer = null;
        try {
            InputStream inputStream = getContentResolver().openInputStream(uri);
            buffer = new byte[inputStream.available()];
            inputStream.read(buffer);
        }catch (Exception e){
            e.printStackTrace();
        }
        return buffer;
    }

    public void save(byte [] data,String path){
        FileOutputStream fout;
        try {
            fout = new FileOutputStream(new File(path));
            fout.write(data);
            fout.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }


    public void onSave(View v) {
        boolean br = EncodeStr2File(edInfo.getText().toString(),strData_Path);
        String strTips;
        if(br){
            strTips="已保存";
        }else{
            strTips = "保存失败";
        }

        Toast tips=Toast.makeText(this,strTips,Toast.LENGTH_SHORT);
        tips.setGravity(Gravity.CENTER,0,0);
        tips.show();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        Uri uri=null;
        if (resultCode == RESULT_OK) {
            uri = data.getData();
        }

        if (requestCode == 1 && uri !=null) {//load
            byte [] datar = readUri(uri);
            if(datar.length>0) {
                save(datar, strData_Path);

                //加载现有内容
                String info = DecodeFile2Str(strData_Path);
                edInfo.setText(info);
            }
        }
    }

    /**
     * 专为Android4.4设计的从Uri获取文件绝对路径，以前的方法已不好使
     */
    @SuppressLint("NewApi")
    public String getPath(final Context context, final Uri uri) {

        final boolean isKitKat = Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT;

        // DocumentProvider
        if (isKitKat && DocumentsContract.isDocumentUri(context, uri)) {
            // ExternalStorageProvider
            if (isExternalStorageDocument(uri)) {
                final String docId = DocumentsContract.getDocumentId(uri);
                final String[] split = docId.split(":");
                final String type = split[0];

                if ("primary".equalsIgnoreCase(type)) {
                    return Environment.getExternalStorageDirectory() + "/" + split[1];
                }
            }
            // DownloadsProvider
            else if (isDownloadsDocument(uri)) {

                final String id = DocumentsContract.getDocumentId(uri);
                final Uri contentUri = ContentUris.withAppendedId(
                        Uri.parse("content://downloads/public_downloads"), Long.valueOf(id));

                return getDataColumn(context, contentUri, null, null);
            }
            // MediaProvider
            else if (isMediaDocument(uri)) {
                final String docId = DocumentsContract.getDocumentId(uri);
                final String[] split = docId.split(":");
                final String type = split[0];

                Uri contentUri = null;
                if ("image".equals(type)) {
                    contentUri = MediaStore.Images.Media.EXTERNAL_CONTENT_URI;
                } else if ("video".equals(type)) {
                    contentUri = MediaStore.Video.Media.EXTERNAL_CONTENT_URI;
                } else if ("audio".equals(type)) {
                    contentUri = MediaStore.Audio.Media.EXTERNAL_CONTENT_URI;
                }

                final String selection = "_id=?";
                final String[] selectionArgs = new String[]{split[1]};

                return getDataColumn(context, contentUri, selection, selectionArgs);
            }
        }
        // MediaStore (and general)
        else if ("content".equalsIgnoreCase(uri.getScheme())) {
            return getDataColumn(context, uri, null, null);
        }
        // File
        else if ("file".equalsIgnoreCase(uri.getScheme())) {
            return uri.getPath();
        }
        return null;
    }

    /**
     * Get the value of the data column for this Uri. This is useful for
     * MediaStore Uris, and other file-based ContentProviders.
     *
     * @param context       The context.
     * @param uri           The Uri to query.
     * @param selection     (Optional) Filter used in the query.
     * @param selectionArgs (Optional) Selection arguments used in the query.
     * @return The value of the _data column, which is typically a file path.
     */
    public String getDataColumn(Context context, Uri uri, String selection,
                                String[] selectionArgs) {

        Cursor cursor = null;
        final String column = "_data";
        final String[] projection = {column};

        try {
            cursor = context.getContentResolver().query(uri, projection, selection, selectionArgs,
                    null);
            if (cursor != null && cursor.moveToFirst()) {
                final int column_index = cursor.getColumnIndexOrThrow(column);
                return cursor.getString(column_index);
            }
        } finally {
            if (cursor != null)
                cursor.close();
        }
        return null;
    }

    /**
     * @param uri The Uri to check.
     * @return Whether the Uri authority is ExternalStorageProvider.
     */
    public boolean isExternalStorageDocument(Uri uri) {
        return "com.android.externalstorage.documents".equals(uri.getAuthority());
    }

    /**
     * @param uri The Uri to check.
     * @return Whether the Uri authority is DownloadsProvider.
     */
    public boolean isDownloadsDocument(Uri uri) {
        return "com.android.providers.downloads.documents".equals(uri.getAuthority());
    }

    /**
     * @param uri The Uri to check.
     * @return Whether the Uri authority is MediaProvider.
     */
    public boolean isMediaDocument(Uri uri) {
        return "com.android.providers.media.documents".equals(uri.getAuthority());
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native boolean CreateKey(String path,String key);
    public native boolean TestKey(String path,String key);
    public native boolean IsKeyFileValid(String path);
    public native boolean FileOpt(String filein,String fileout,boolean isEncode);
    public native boolean EncodeStr2File(String strin,String fileout);
    public native String DecodeFile2Str(String filein);
}
