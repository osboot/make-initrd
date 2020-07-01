SYSLOG_DATADIR	= $(FEATURESDIR)/syslog/data
SYSLOG_PROGS	= klogd syslogd
SYSLOG_FILES	= $(wildcard $(SYSCONFDIR)/sysconfig/klogd $(SYSCONFDIR)/sysconfig/syslogd /var/lib/klogd)

