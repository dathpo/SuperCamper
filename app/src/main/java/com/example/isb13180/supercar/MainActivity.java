package com.example.isb13180.supercar;

import android.os.AsyncTask;
import android.os.Bundle;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
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
import android.widget.ListView;

import retrofit2.Call;

public class MainActivity extends AppCompatActivity {
    private ListView mList;
    private ArrayList<String> arrayList;

    private TcpClient mTcpClient = null;
    private ConnectTask connecttask = null;
    private String ipAddressOfServerDevice;

    Button b1,b2;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        ipAddressOfServerDevice = "192.168.4.1";

        arrayList = new ArrayList<String>();

        connecttask = new ConnectTask();
        connecttask.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);

        b1 = findViewById(R.id.button_left);
        b2 = findViewById(R.id.button_right);

        b1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                //sends the message to the server
                while(view.isSelected()==true) {
                    if (mTcpClient != null) {
                        try {
                            System.out.println("Left");
                            mTcpClient.sendMessage("Z");
                        } catch (Exception e) {
                            Log.e("TCP", "exception", e);
                        }
                    }
                }
            }
        });
        b2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                //sends the message to the server
                if (mTcpClient != null) {
                    try {
                        System.out.println("Right");
                        mTcpClient.sendMessage("Q");
                    } catch (Exception e) {
                        Log.e("TCP", "exception", e);
                    }
                }
            }
        });
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
