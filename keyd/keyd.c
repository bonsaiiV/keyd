/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#define _GNU_SOURCE
#include <assert.h>
#include <getopt.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>


#include "keyd.h"
#include "projconf.h"
#include "config.h"
#include "log.h"
#include "keys.h"

int panic_check_enabled = 0;
char* config_path = CONFIG_DIR;

static int ipc_exec(int type, const char *data, size_t sz, uint32_t timeout)
{
	struct ipc_message msg;

	assert(sz <= sizeof(msg.data));

	msg.type = type;
	msg.sz = sz;
	msg.timeout = timeout;
	memcpy(msg.data, data, sz);

	int con = ipc_connect();

	if (con < 0) {
		perror("connect");
		exit(-1);
	}

	xwrite(con, &msg, sizeof msg);
	xread(con, &msg, sizeof msg);

	if (msg.sz) {
		xwrite(1, msg.data, msg.sz);
		xwrite(1, "\n", 1);
	}

	return msg.type == IPC_FAIL;

	return type == IPC_FAIL;
}

static int version(int argc, char *argv[])
{
	UNUSED(argc), UNUSED(argv);
	printf("keyd " VERSION "\n");

	return 0;
}

static int help(int argc, char *argv[])
{
	UNUSED(argc), UNUSED(argv);
	printf("usage: keyd [-v] [-h] [command] [<args>]\n\n"
		"Commands:\n"
		"    monitor [-t]                   Print key events in real time.\n"
		"    list-keys                      Print a list of valid key names.\n"
		"    reload                         Trigger a reload .\n"
		"    listen                         Print layer state changes of the running keyd daemon to stdout.\n"
		"    bind <binding> [<binding>...]  Add the supplied bindings to all loaded configs.\n"
		"    check [-c CONFIG]              Checks wheather config can be parsed. Alternate config can be specified.\n"
		"Options:\n"
		"    -v, --version      Print the current version and exit.\n"
		"    -h, --help         Print help and exit.\n");

	return 0;
}

static int list_keys(int argc, char *argv[])
{
	UNUSED(argc), UNUSED(argv);
	size_t i;

	for (i = 0; i < 256; i++) {
		const char *altname = keycode_table[i].alt_name;
		const char *shiftedname = keycode_table[i].shifted_name;
		const char *name = keycode_table[i].name;

		if (name)
			printf("%s\n", name);
		if (altname)
			printf("%s\n", altname);
		if (shiftedname)
			printf("%s\n", shiftedname);
	}

	return 0;
}


static int add_bindings(int argc, char *argv[])
{
	int i;
	int ret = 0;

	for (i = 1; i < argc; i++) {
		if (ipc_exec(IPC_BIND, argv[i], strlen(argv[i]), 0))
			ret = -1;
	}

	if (!ret)
		printf("Success\n");

	return ret;
}

static void read_input(int argc, char *argv[], char *buf, size_t *psz)
{
	size_t sz = 0;
	size_t bufsz = *psz;

	if (argc != 0) {
		int i;
		for (i = 0; i < argc; i++) {
			sz += snprintf(buf+sz, bufsz-sz, "%s%s", argv[i], i == argc-1 ? "" : " ");

			if (sz >= bufsz)
				die("maximum input length exceeded");
		}
	} else {
		while (1) {
			size_t n;

			if ((n = read(0, buf+sz, bufsz-sz)) <= 0)
				break;
			sz += n;

			if (bufsz == sz)
				die("maximum input length exceeded");
		}
	}

	*psz = sz;
}

static int check_config(int argc, char *argv[])
{
	UNUSED(argc), UNUSED(argv);
	char err_message[128];
	struct config dummy;
	if(access(config_path, F_OK)){
		snprintf(err_message, sizeof err_message, "no config found at \"%s\"", config_path);
		perror(err_message);
		return 1;
	}
	if(access(config_path, R_OK)){
		snprintf(err_message, sizeof err_message, "config at \"%s\" can not be read", config_path);
		perror(err_message);
		return 1;
	}
	struct stat inode;
	stat(config_path, &inode);
	if (S_ISREG(inode.st_mode))
	{
		if(config_parse(&dummy, config_path)){
			fprintf(stderr, "failed to parse config at \"%s\": %s\n", config_path, errstr);
			return -1;
		}
		printf("config at \"%s\" successfully parsed\n", config_path);
		return 0;
	} else if (S_ISDIR(inode.st_mode)){
		DIR *dir_handel;
		struct dirent *dirent;

		if (!(dir_handel = opendir(CONFIG_DIR))) {
			snprintf(err_message, sizeof err_message, "failed to open directory \"%s\"", config_path);
			perror(err_message);
			return -1;
		}

		while ((dirent = readdir(dir_handel))) {
			char path[1024];
			int len;

			len = snprintf(path, sizeof path, "%s/%s", CONFIG_DIR, dirent->d_name);
			if (stat(path, &inode) != -1)
				continue;

			if (!S_ISREG(inode.st_mode))
				continue;


			if (len >= 5 && !strcmp(path + len - 5, ".conf")) {
				keyd_log("CONFIG: parsing b{%s}\n", path);

				if (!config_parse(&dummy, path)) {
					continue;
				} else {
					keyd_log("failed to parse config at \"%s\": %s\n", config_path, errstr);
				}

			}
		}

		closedir(dir_handel);
	}
	return 0;

}

static int cmd_do(int argc, char *argv[])
{
	char buf[MAX_IPC_MESSAGE_SIZE];
	size_t sz = sizeof buf;
	uint32_t timeout = 0;

	if (argc > 2 && !strcmp(argv[1], "-t")) {
		timeout = atoi(argv[2]);
		argc -= 2;
		argv += 2;
	}

	read_input(argc-1, argv+1, buf, &sz);

	return ipc_exec(IPC_MACRO, buf, sz, timeout);
}


static int input(int argc, char *argv[])
{
	char buf[MAX_IPC_MESSAGE_SIZE];
	size_t sz = sizeof buf;
	uint32_t timeout = 0;

	if (argc > 2 && !strcmp(argv[1], "-t")) {
		timeout = atoi(argv[2]);
		argc -= 2;
		argv += 2;
	}

	read_input(argc-1, argv+1, buf, &sz);

	return ipc_exec(IPC_INPUT, buf, sz, timeout);
}

static int layer_listen(int argc, char *argv[])
{
	UNUSED(argc), UNUSED(argv);
	struct ipc_message msg = {0};

	int con = ipc_connect();

	if (con < 0) {
		perror("connect");
		exit(-1);
	}

	msg.type = IPC_LAYER_LISTEN;
	xwrite(con, &msg, sizeof msg);

	while (1) {
		char buf[512];
		ssize_t sz = read(con, buf, sizeof buf);

		if (sz <= 0)
			return -1;

		xwrite(1, buf, sz);
	}
}

static int reload(int argc, char **argv)
{
	UNUSED(argc), UNUSED(argv);
	ipc_exec(IPC_RELOAD, NULL, 0, 0);

	return 0;
}

struct {
	const char *name;

	int (*fn)(int argc, char **argv);
} commands[] = {
	{"help", help},
	{"version", version},

	/* Keep -e and -m for backward compatibility. TODO: remove these at some point. */
	{"monitor", monitor},
	{"check", check_config},
	{"bind", add_bindings},
	{"input", input},
	{"do", cmd_do},

	{"listen", layer_listen},

	{"reload", reload},
	{"list-keys", list_keys},
};

int main(int argc, char *argv[])
{
	size_t i;

	log_level =
	    atoi(getenv("KEYD_DEBUG") ? getenv("KEYD_DEBUG") : "");

	if (isatty(1))
		suppress_colours = getenv("NO_COLOR") ? 1 : 0;
	else
		suppress_colours = 1;

	dbg("Debug mode activated");

	signal(SIGTERM, exit);
	signal(SIGINT, exit);
	signal(SIGPIPE, SIG_IGN);
	static struct option longopts[] = {
		{"help", 0, 0, 'h'},
		{"version", 0, 0, 'v'},
		{"config", 1, 0, 'c'},
		{"monitor", 0, 0, 'm'},
		{"expression", 0, 0, 'e'},
		{"enable-panic", 0, 0, 'p'},
		{0, 0, 0, 0},
	};

	int (*cmd)(int,char**)  = &run_daemon;
	if (argc > 1) {
		for (i = 0; i < ARRAY_SIZE(commands); i++)
			if (!strcmp(commands[i].name, argv[1])) {
				cmd = commands[i].fn;
				argc--;
				argv++;
				break;
			}
	}
	char opt;
	while ((opt = getopt_long(argc, argv, "c:hvpme", longopts, NULL)) != -1){
		switch (opt){
			case 'h':
				help(argc, argv);
				break;
			case 'v':
				version(argc, argv);
				break;
			case 'c':
				config_path = optarg;
				break;
			case 'p':
				panic_check_enabled = 1;
				break;
			case 'm':
				fprintf(stderr, "Options -m and --monitor are deprecated. Instead use monitor as subcommand.");
				cmd = &monitor;
				break;
			case 'e':
				fprintf(stderr, "Options -e and --expression are deprecated. Instead use bind as subcommand.");
				cmd = &add_bindings;
				break;
			default:
				cmd = &help;
		}
	}

	cmd(argc, argv);
}
