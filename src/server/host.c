#include <errno.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

static int get_host_name_from_sockaddr(const struct sockaddr* addr, char* out)
{
	char host[NI_MAXHOST];
	int addrlen = 0;
	int ret = 0;

	addrlen = (AF_INET == addr->sa_family)
		? sizeof(struct sockaddr_in)
		: sizeof(struct sockaddr_in6);
	ret = getnameinfo(
			addr,
			addrlen,
			host,
			NI_MAXHOST,
			0,
			0,
			NI_NUMERICHOST);
	if (0 != ret) {
		fprintf(
			stderr,
			"getnameinfo() failed: %s\n",
			gai_strerror(ret));
		return -1;
	}

	strncpy(out, host, NI_MAXHOST);
}

int get_host_of_first_nonloopback_device(char* out)
{
	struct ifaddrs* addrs_head;
	struct ifaddrs* addrs;
	int family = 0;
	int err = 0;
	int ret = 0;

	ret = getifaddrs(&addrs_head);
	if (0 != ret) {
		err = errno;
		fprintf(
			stderr,
			"Failed to get network interface structs (%d) %s\n",
			err,
			strerror(err));
		return -1;
	}

	addrs = addrs_head;
	while (1) {
		if (0 == addrs) {
			break;
		}

		if (0 == addrs->ifa_addr) {
			addrs = addrs->ifa_next;
			continue;
		}

		if (0 == strncmp(addrs->ifa_name, "lo", 2)) {
			addrs = addrs->ifa_next;
			continue;
		}

		family = addrs->ifa_addr->sa_family;
		if (AF_INET == family || AF_INET6 == family) {
			ret = get_host_name_from_sockaddr(addrs->ifa_addr, out);
			if (0 != ret) {
				freeifaddrs(addrs_head);
				return -1;
			}
			break;
		}

		addrs = addrs->ifa_next;
	}

	freeifaddrs(addrs_head);

	return 0;
}
