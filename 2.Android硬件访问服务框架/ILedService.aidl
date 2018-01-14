package android.os;

/** {@hide} */
interface IVibratorService
{
/*    boolean hasVibrator();
    void vibrate(int uid, String opPkg, long milliseconds, int usageHint, IBinder token);
    void vibratePattern(int uid, String opPkg, in long[] pattern, int repeat, int usageHint, IBinder token);
    void cancelVibrate(IBinder token);
*/

  int ledCtrl(int which, int status);
}
