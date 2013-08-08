#
# Regular cron jobs for the dandelion package
#
0 4	* * *	root	[ -x /usr/bin/dandelion_maintenance ] && /usr/bin/dandelion_maintenance
