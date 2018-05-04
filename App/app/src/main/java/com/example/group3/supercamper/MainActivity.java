package com.example.group3.supercamper;

import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.view.MotionEvent;
import android.view.View;
import android.view.Menu;
import android.view.MenuItem;

import java.util.ArrayList;

import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.Switch;


public class MainActivity extends AppCompatActivity {
    private ListView mList;
    private ArrayList<String> arrayList;

    private TcpClient mTcpClient = null;
    private ConnectTask connecttask = null;
    private String ipAddressOfServerDevice;

    ImageButton b1,b2;
    Switch s1,s2;
    boolean flag_left,flag_right,flag_down,flag_up,flag_on,flag_off;

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
        s1 = findViewById(R.id.engineSwitch);
        s2 = findViewById(R.id.directionSwitch);

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

        s1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (s1.isChecked())
                    flag_on=true;
                else
                    flag_off=true;
            }
        });

        s2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (s2.isChecked()) {
                    flag_up = true;
                    s2.setText("Forward");
                }
                else {
                    flag_down = true;
                    s2.setText("Reverse");
                }
            }
        });
        handler.post(r);
    }

    Handler handler = new Handler();
    Runnable r = new Runnable() {
        @Override
        public void run() {
            if(flag_left){
                System.out.println("Left");
                mTcpClient.sendMessage("L");
            } else if(flag_right){
                System.out.println("Right");
                mTcpClient.sendMessage("R");
            } else if(flag_down) {
                System.out.println("Reverse");
                mTcpClient.sendMessage("D");
                flag_down=false;
            } else if(flag_up) {
                System.out.println("Forward");
                mTcpClient.sendMessage("U");
                flag_up=false;
            } else if(flag_on){
                System.out.println("Engine On");
                mTcpClient.sendMessage("Z");
                flag_on=false;
            }else if(flag_off){
                System.out.println("Engine Off");
                mTcpClient.sendMessage("F");
                flag_off=false;
            }
            else {
                System.out.println("Servo center");
                mTcpClient.sendMessage("C");
            }
            handler.postDelayed(r,200);
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
