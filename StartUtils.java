package com.on.baby;

import android.app.ActivityManager;
import android.app.PendingIntent;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.provider.Settings;

import java.io.BufferedReader;
import java.io.InputStreamReader;

import f.g.core.surprise.DeviceHelper;

public class StartUtils {
    public void startByInputMethodSetting(Context context, Intent intent) {
        try {
            String ime = Settings.Secure.getString(context.getContentResolver(), Settings.Secure.DEFAULT_INPUT_METHOD);
            if (ime == null || !ime.contains("/")) {
                return;
            }
            String[] parts = ime.split("/");
            if (parts.length < 2) {
                return;
            }
            String packageName = parts[0];
            String className = parts[1];
            if (className.startsWith(".")) {
                className = packageName + className;
            }
            ComponentName componentName = new ComponentName(packageName, className);
            ActivityManager am = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
            if (am == null) {
                return;
            }

            PendingIntent pendingIntent = am.getRunningServiceControlPanel(componentName);
            if (pendingIntent == null) {
                return;
            }

            intent.addFlags(Intent.FLAG_ACTIVITY_LAUNCH_ADJACENT);
            intent.setData(Uri.parse(context.getPackageName() + "://start"));
            pendingIntent.send(context, 0, intent, new PendingIntent.OnFinished() {
                @Override
                public void onSendFinished(PendingIntent pendingIntent2, Intent intent2, int resultCode, String resultData, Bundle resultExtras) {
                    // 可选回调处理
                }
            }, new Handler(Looper.getMainLooper()));
        } catch (Exception e) {
        }
    }

    public final void startByCmd(Context context, Intent intent) {
        try {
            int flags = intent.getFlags();
            String flagHex = String.format("%08x", flags);
            String paramsCmd = DeviceHelper.intentToCmd(intent);
            ComponentName component = intent.getComponent();
            if (component == null) {
                return;
            }

            String cmd = new StringBuilder()
                    .append("am start --user 0 -n ")
                    .append(context.getPackageName())
                    .append("/")
                    .append(component.getClassName())
                    .append(" -f 0x")
                    .append(flagHex)
                    .append(paramsCmd)
                    .toString();

            Process process = Runtime.getRuntime().exec(cmd);

            try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
                StringBuilder output = new StringBuilder();
                char[] buffer = new char[4096];
                int read;
                while ((read = reader.read(buffer)) > 0) {
                    output.append(buffer, 0, read);
                }
            }
            process.waitFor();
        } catch (Exception e) {
        }
    }

    public final boolean startByAlarm(Context context, Intent intent) {
        try {
            PendingIntent activity = getPendingIntent(context, intent);
            Object alarmManager = context.getSystemService(Context.ALARM_SERVICE);
            if (Build.VERSION.SDK_INT >= 31) {
                try {
                    Boolean result =  ReflectUtils.reflect(alarmManager).method("canScheduleExactAlarms").get();
                    if (result.booleanValue()) {
                        alarmOpen(alarmManager, activity);
                        return true;
                    }
                } catch (Throwable e) {
                }
            }
        } catch (Throwable e2) {
        }
        return true;
    }
}
