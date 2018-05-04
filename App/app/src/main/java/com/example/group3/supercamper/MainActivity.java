package com.example.group3.supercamper;

import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.view.MotionEvent;
import android.view.View;
import android.view.Menu;
import android.view.MenuItem;


import android.widget.ImageButton;
import android.widget.Switch;


public class MainActivity extends AppCompatActivity {

    private TcpClient mTcpClient = null;
    private ConnectTask connecttask = null;
    private String ipAddressOfServerDevice;
    private String lastMessage="";

    ImageButton b1,b2;
    Switch s1,s2;
    boolean flag_left,flag_right,flag_down,flag_up,flag_on,flag_off,flag_center;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        ipAddressOfServerDevice = "192.168.4.1";
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
                    flag_center=true;
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
                    flag_center=true;
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
                sendMessage("L");
            } else if(flag_right){
                sendMessage("R");
            } else if(flag_down) {
                sendMessage("D");
                flag_down=false;
            } else if(flag_up) {
                sendMessage("U");
                flag_up=false;
            } else if(flag_on){
                sendMessage("Z");
                flag_on=false;
            }else if(flag_off){
                sendMessage("F");
                flag_off=false;
            }
            else if(flag_center){
                sendMessage("C");
                flag_center=false;
            }
            handler.postDelayed(r,100);
        }
    };

    private void sendMessage(String s){
        if (!s.equals(lastMessage)) {
            lastMessage = s;
            System.out.println(s);
            mTcpClient.sendMessage(s);
        }
    }
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
