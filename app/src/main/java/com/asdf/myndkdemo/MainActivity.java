package com.asdf.myndkdemo;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

public class MainActivity extends Activity implements View.OnClickListener {
    Button btn1,btn2,btn3,btn4,btn5;
    DataProvider dataProvider;
    // Used to load the 'native-lib' library on application startup.


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        btn1=(Button) findViewById(R.id.btn1);
        btn2=(Button) findViewById(R.id.btn2);
        btn3=(Button) findViewById(R.id.btn3);
        btn4=(Button) findViewById(R.id.btn4);
        btn5= (Button) findViewById(R.id.btn5);
        btn1.setOnClickListener(this);
        btn2.setOnClickListener(this);
        btn3.setOnClickListener(this);
        btn4.setOnClickListener(this);
        btn5.setOnClickListener(this);
        dataProvider=new DataProvider();
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn1:
                int rint=dataProvider.add(3, 5);
                Toast.makeText(this, "相加的结果是"+rint, Toast.LENGTH_SHORT).show();
                break;
            case R.id.btn2:
                String rs=dataProvider.sayHelloInC("TBY");
                Toast.makeText(this, "返回字符串是"+rs, Toast.LENGTH_SHORT).show();
                break;
            case R.id.btn3:
                int[] arrs={1,2,3,4,5,6};
                dataProvider.intMethod(arrs);
                for (int i = 0; i < arrs.length; i++) {
                    Log.d("arras["+i+"]", arrs[i]+"");
                }
                break;
            case R.id.btn4:
                int rintsub=DataProvider.sub(5, 3);
                Toast.makeText(this, "相减的结果是"+rintsub, Toast.LENGTH_SHORT).show();
                break;
            case R.id.btn5:
                dataProvider.throwingMethod();
                break;
            default:
                break;
        }
    }

}
