#define LOG_TAG "RilWrapper"
#define RIL_SHLIB
#include <telephony/ril_cdma_sms.h>
#include <sys/system_properties.h>
#include <telephony/librilutils.h>
#include <cutils/sockets.h>
#include <telephony/ril.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/cdefs.h>
#include <utils/Log.h>
#include <sys/stat.h>
#include <pthread.h>
#include <termios.h>
#include <alloca.h>
#include <assert.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>

#define REAL_RIL_NAME				"/system/lib/libsec-ril.so"


static RIL_RadioFunctions const *mRealRadioFuncs;
static const struct RIL_Env *mEnv;

const RIL_RadioFunctions* RIL_Init(const struct RIL_Env *env, int argc, char **argv)
{
	RIL_RadioFunctions const* (*fRealRilInit)(const struct RIL_Env *env, int argc, char **argv);
	static RIL_RadioFunctions rilInfo;
	void *realRilLibHandle;
	int i;


	//save the env;
	mEnv = env;

	//get the real RIL
	realRilLibHandle = dlopen(REAL_RIL_NAME, RTLD_LOCAL);
	if (!realRilLibHandle) {
		RLOGE("Failed to load the real RIL '" REAL_RIL_NAME  "': %s\n", dlerror());
		return NULL;
	}

	//remove "-c" command line as Samsung's RIL does not understand it - it just barfs instead
	for (i = 0; i < argc; i++) {
		if (!strcmp(argv[i], "-c") && i != argc -1) {	//found it
			memcpy(argv + i, argv + i + 2, sizeof(char*[argc - i - 2]));
			argc -= 2;
		}
	}

	//load the real RIL
	fRealRilInit = dlsym(realRilLibHandle, "RIL_Init");
	if (!fRealRilInit) {
		RLOGE("Failed to find the real RIL's entry point\n");
		goto out_fail;
	}

	RLOGD("Calling the real RIL's entry point with %u args\n", argc);
	for (i = 0; i < argc; i++)
		RLOGD("  argv[%2d] = '%s'\n", i, argv[i]);

	//try to init the real ril
	mRealRadioFuncs = fRealRilInit(env, argc, argv);
	if (!mRealRadioFuncs) {
		RLOGE("The real RIL's entry point failed\n");
		goto out_fail;
	}

	//copy the real RIL's info struct, then replace the onRequest pointer with our own
	rilInfo = *mRealRadioFuncs;

	RLOGD("Wrapped RIL version is '%s'\n", mRealRadioFuncs->getVersion());

	//we're all good - return to caller
	return &rilInfo;

out_fail:
	dlclose(realRilLibHandle);
	return NULL;
}
