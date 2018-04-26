/*
 * 1.getService
 * 2. call Service's sayhello sayhello_to
*/

/*
 * test_client <hello|goodbye> name
 * */
import android.util.Slog;
import android.os.ServiceManager;
import android.os.IBinder;

public class TestClient {
    private static final String TAG = "HelloService";
    
    public static void main(String args[]) {
        if (args.length == 0) {
            System.out.println("Usage:need params:<hello|goodbye> name");
            return;
        }

        if (args[0].equals("hello")) {
            IBinder binder = ServiceManager.getService("hello");
            if (binder == null) {
                System.out.println("cannot get hello Service");
                Slog.i(TAG, "cannot get hello Service");
                return;
            }
                
            IHelloService svr = IHelloService.Stub.asInterface(binder);

            if(args.length == 1) {
                try {
                    svr.sayhello();
                    System.out.println("call sayhello");
                    Slog.i(TAG, "call sayhello");
                } catch (Exception e) {

                }
            } else {
                try {
                    int cnt = svr.sayhello_to(args[1]);
                    System.out.println("call sayhello_to "+args[1]+":cnt2 = "+cnt);
                    Slog.i(TAG, "call sayhello_to "+args[1]+":cnt2 = "+cnt);
                } catch (Exception e) {

                }
            }
        }
    }
}
