package FTAGUI;

import javax.swing.*;
import javax.swing.event.*;
import javax.swing.text.*;
import javax.swing.border.*;
import javax.accessibility.*;

import java.awt.*;
import java.awt.event.*;
import java.beans.*;
import java.util.*;
import java.io.*;
import java.applet.*;
import java.net.*;
import java.lang.*;

class RestrictedLengthExclusiveDocument extends PlainDocument {

    /** This method overrides the insertString method to ensure that only
     * characters that do not exist in the excludes string are accepted.
     * @param offs The offset into the text field to insert the string
     * @param str The string to insert into the Text Field
     * @param a The attributes to associate with the inserted content. This may
     * be null if there are no attributes.
     * @throws BadLocationException If the string to insert is null.
     */

    private int _length;
    private String _excludes;

    public RestrictedLengthExclusiveDocument(int length, String excludes) {
	super();
	_excludes = excludes;
	_length = length;
    }

    public void insertString(int offs, String str, AttributeSet a)

	throws BadLocationException {
	if (str == null) {
	    return;
	}
	// Get the current text in the Text Field
	String currentText = super.getText(0, super.getLength());

	// Find number of characters in the string to insert
	char[] charIn = str.toCharArray();

	for (int i = 0; i < charIn.length; i++) {

	    if(currentText.length() < _length) {

		//If the character does not appear in excludes
		if(_excludes.indexOf(charIn[i]) < 0) {
		    super.insertString(offs+i, new String(charIn, i, 1), a);
		}
	    }
	}
    }
}