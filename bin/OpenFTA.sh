#!/bin/sh

export "CLASSPATH=FTAGUI.jar;images.jar"

"$JAVA_HOME/bin/java" -Xmx256m FTAGUI.OpenFTA
