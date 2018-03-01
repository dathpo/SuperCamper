package com.example.isb13180.supercar;

import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.MotionEvent;
import android.view.View;
import android.view.Menu;
import android.view.MenuItem;
import android.app.Activity;
import android.os.Bundle;

import android.view.View;
import android.util.Log;
import java.util.ArrayList;

import android.os.AsyncTask;

import android.widget.Button;
import android.widget.ImageButton;
import android.widget.ListView;

import retrofit2.Call;

public class MainActivity extends AppCompatActivity {
    private ListView mList;
    private ArrayList<String> arrayList;

    private TcpClient mTcpClient = null;
    private ConnectTask connecttask = null;
    private String ipAddressOfServerDevice;

    ImageButton b1,b2;
    boolean flag_left,flag_right,flag_center;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        ipAddressOfServerDevice = "192.168.4.1";

        arrayList = new ArrayList<String>();

        connecttask = new ConnectTask();
        connecttask.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);

        b1 = findViewById(R.id.leftArrow);
        b2 = findViewById(R.id.rightArrow);

        b1.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    flag_left = true;
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    flag_left = false;
                }
                return false;
            }
        });

        b2.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View view, MotionEvent event) {
                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    flag_right = true;
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    flag_right = false;
                }
                return false;
            }
        });
        handler.post(r);
    }

    Handler handler = new Handler();
    Runnable r = new Runnable() {
        @Override
        public void run() {
            if(flag_left){System.out.println("Right");}
            if(flag_right){System.out.println("Left");}
            handler.postDelayed(r,100);
        }
    };



    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    public class ConnectTask extends AsyncTask<String, String, TcpClient> {

        @Override
        protected TcpClient doInBackground(String... message) {

            mTcpClient = new TcpClient(new TcpClient.OnMessageReceived() {
                @Override
                public void messageReceived(String message) {
                    try {
                        publishProgress(message);
                        if (message != null) {
                            System.out.println("Returned message from socket::::: >>>>>>" + message);
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
            }, ipAddressOfServerDevice);
            mTcpClient.run();
            if (mTcpClient != null) {
                mTcpClient.sendMessage("Initial message when connected with Socket Server");
            }
            return null;
        }
    }

    @Override
    protected void onDestroy()
    {
        try{
            System.out.println("onDestroy.");
            mTcpClient.sendMessage("bye");
            mTcpClient.stopClient();
            connecttask.cancel(true);
            connecttask = null;
        }catch (Exception e)
        {
            e.printStackTrace();
        }
        super.onDestroy();
    }
}
