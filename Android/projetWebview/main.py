import android
import sys
 
droid = android.Android()
droid.webViewShow('file:///storage/emulated/0/com.hipipal.qpyplus/projects3/projetWebview/index.html')

while True:
    event = droid.eventWait().result

    if event['name'] == 'donnee':
        message = "Coucou, " + str(event['data'])
        droid.makeToast(message)
