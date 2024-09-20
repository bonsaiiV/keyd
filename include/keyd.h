/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#ifndef KEYD_H_
#define KEYD_H_

#ifdef __FreeBSD__
	#include <dev/evdev/input.h>
#else
	#include <linux/input.h>
#endif

#include "device.h"

#define MAX_IPC_MESSAGE_SIZE 4096

#define ARRAY_SIZE(x) (int)(sizeof(x)/sizeof(x[0]))
#define VKBD_NAME "keyd virtual keyboard"
#define UNUSED(x) (void)x

enum event_type {
	EV_DEV_ADD,
	EV_DEV_REMOVE,
	EV_DEV_EVENT,
	EV_FD_ACTIVITY,
	EV_FD_ERR,
	EV_TIMEOUT,
};

struct event {
	enum event_type type;
	struct device *dev;
	struct device_event *devev;
	int timestamp;
	int fd;
};

struct ipc_message {
	enum {
		IPC_SUCCESS,
		IPC_FAIL,

		IPC_BIND,
		IPC_INPUT,
		IPC_MACRO,
		IPC_RELOAD,
		IPC_LAYER_LISTEN,
	} type;
	
	uint32_t timeout;
	char data[MAX_IPC_MESSAGE_SIZE];
	size_t sz;
};

int monitor(int argc, char *argv[]);
int run_daemon(int argc, char *argv[]);

void evloop_add_fd(int fd);
int evloop(int (*event_handler) (struct event *ev));

void xwrite(int fd, const void *buf, size_t sz);
void xread(int fd, void *buf, size_t sz);

int ipc_create_server(void);
int ipc_connect(void);

extern struct device device_table[MAX_DEVICES];
extern size_t device_table_sz;
extern int panic_check_enabled;

#endif
