#!/usr/bin/env python3
"""Spike #50 — the daemon's side of the recommended cut: it hears exactly one
Origin() per capture and nothing else."""
import sys
import time

import dbus
import dbus.service
from dbus.mainloop.glib import DBusGMainLoop
from gi.repository import GLib

LOG = sys.argv[1]


class Sink(dbus.service.Object):
    @dbus.service.method("org.denkzettel.Spike", in_signature="ss")
    def Origin(self, caption, app_id):
        with open(LOG, "a", encoding="utf8") as f:
            f.write("%s  Origin(caption=%r, app=%r)\n"
                    % (time.strftime("%H:%M:%S"), str(caption), str(app_id)))


DBusGMainLoop(set_as_default=True)
bus = dbus.SessionBus()
name = dbus.service.BusName("org.denkzettel.Spike", bus)
Sink(bus, "/Sink")
with open(LOG, "a", encoding="utf8") as f:
    f.write("%s  sink up\n" % time.strftime("%H:%M:%S"))
GLib.MainLoop().run()
