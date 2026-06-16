package com.mendelu.fordil;

import android.os.Bundle;
import androidx.core.view.WindowCompat;
import org.qtproject.qt.android.bindings.QtActivity;

public class MainActivity extends QtActivity {

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // WindowCompat.enableEdgeToEdge(getWindow());
    }
}
