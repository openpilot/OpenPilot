#
# Regular cron jobs for the openpilot package
#
0 4	* * *	root	[ -x /usr/bin/openpilot_maintenance ] && /usr/bin/openpilot_maintenance
