
#include "keep.h"
#include "logutils.h"

jboolean Keep::bringToTop(JNIEnv *env, jclass clazz, jobject context) {
    jboolean result = JNI_FALSE;

    if (context == nullptr) {
        LOGE("Context is null");
        return JNI_FALSE;
    }

    // 获取 Context.ACTIVITY_SERVICE 常量
    jclass contextClass = env->GetObjectClass(context);
    jfieldID ACTIVITY_SERVICE_fid = env->GetStaticFieldID(contextClass,
                                                          "ACTIVITY_SERVICE",
                                                          "Ljava/lang/String;");
    jstring activityService = (jstring) env->GetStaticObjectField(contextClass, ACTIVITY_SERVICE_fid);

    // 调用 context.getSystemService(Context.ACTIVITY_SERVICE)
    jmethodID getSystemService_mid = env->GetMethodID(contextClass,
                                                      "getSystemService",
                                                      "(Ljava/lang/String;)Ljava/lang/Object;");
    jobject activityManager = env->CallObjectMethod(context, getSystemService_mid, activityService);
    if (activityManager == nullptr) {
        LOGE("Failed to get ActivityManager");
        return JNI_FALSE;
    }

    // ActivityManager.getRunningTasks(int)
    jclass activityManagerClass = env->GetObjectClass(activityManager);
    jmethodID getRunningTasks_mid = env->GetMethodID(activityManagerClass,
                                                     "getRunningTasks",
                                                     "(I)Ljava/util/List;");
    jobject taskList = env->CallObjectMethod(activityManager, getRunningTasks_mid, 10);
    if (taskList == nullptr) {
        LOGE("getRunningTasks returned null");
        return JNI_FALSE;
    }

    // 遍历 List<RunningTaskInfo>
    jclass listClass = env->GetObjectClass(taskList);
    jmethodID size_mid = env->GetMethodID(listClass, "size", "()I");
    jmethodID get_mid = env->GetMethodID(listClass, "get", "(I)Ljava/lang/Object;");
    jint size = env->CallIntMethod(taskList, size_mid);

    // 当前包名
    jmethodID getPackageName_mid = env->GetMethodID(contextClass, "getPackageName", "()Ljava/lang/String;");
    jstring pkgName = (jstring) env->CallObjectMethod(context, getPackageName_mid);

    for (int i = 0; i < size; i++) {
        jobject taskInfo = env->CallObjectMethod(taskList, get_mid, i);
        if (taskInfo == nullptr) continue;

        jclass taskInfoClass = env->GetObjectClass(taskInfo);

        // topActivity: ComponentName
        jfieldID topActivity_fid = env->GetFieldID(taskInfoClass, "topActivity", "Landroid/content/ComponentName;");
        jobject topActivity = env->GetObjectField(taskInfo, topActivity_fid);

        if (topActivity != nullptr) {
            jclass compClass = env->GetObjectClass(topActivity);
            jmethodID getPackageName_mid2 = env->GetMethodID(compClass,
                                                             "getPackageName",
                                                             "()Ljava/lang/String;");
            jstring topPkg = (jstring) env->CallObjectMethod(topActivity, getPackageName_mid2);

            const char *pkgC = env->GetStringUTFChars(pkgName, nullptr);
            const char *topPkgC = env->GetStringUTFChars(topPkg, nullptr);

            if (strcmp(pkgC, topPkgC) == 0) {
                LOGD("Found matching task, bringing to front...");

                // 获取 task id
                jfieldID id_fid = env->GetFieldID(taskInfoClass, "id", "I");
                jint taskId = env->GetIntField(taskInfo, id_fid);

                // 调用 moveTaskToFront
                jmethodID moveTaskToFront_mid = env->GetMethodID(activityManagerClass,
                                                                 "moveTaskToFront",
                                                                 "(II)V");
                jint MOVE_TASK_WITH_HOME = 1; // ActivityManager.MOVE_TASK_WITH_HOME = 1
                env->CallVoidMethod(activityManager, moveTaskToFront_mid, taskId, MOVE_TASK_WITH_HOME);

                result = JNI_TRUE;

                env->ReleaseStringUTFChars(pkgName, pkgC);
                env->ReleaseStringUTFChars(topPkg, topPkgC);
                break;
            }

            env->ReleaseStringUTFChars(pkgName, pkgC);
            env->ReleaseStringUTFChars(topPkg, topPkgC);
        }
    }

    return result;
}