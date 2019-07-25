package com.example.viktorhorvath.rootdetection;

import android.graphics.Color;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import java.io.File;

public class MainActivity extends AppCompatActivity {

    private static final String FILEPATH = "/data/local/tmp/test.txt";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        final TextView label = findViewById(R.id.textResult);

        Button buttonTest = findViewById(R.id.buttonTest);
        buttonTest.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                File file = new File(FILEPATH);
                if(file.exists()) {
                    label.setText(R.string.rooted);
                    label.setTextColor(Color.RED);
                } else {
                    label.setText(R.string.not_rooted);
                    label.setTextColor(Color.GREEN);
                }
            }
        });
    }
}
