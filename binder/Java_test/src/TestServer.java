/**
 * 1.addService
 * 2.while(true) {
 *      read data; parse data; call func; reply
 * }
 */
import android.util.Slog;
import android.os.ServiceManager;

public class TestServer {
    private static final String TAG = "HelloService";
    
    public static void main(String args[]) {
        /*addService*/
        Slog.i(TAG, "add hello Service");
        ServiceManager.addService("hello", new HelloService());
        
        while(true) {
            try {
                Thread.sleep(100);
            } catch (Exception e) {
                
            }
        }
    }
}
