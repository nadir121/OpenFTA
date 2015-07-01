/******************************************************************************
**
** OpenFTA - Fault Tree Analysis
** Copyright (C) 2005 FSC Limited
**
** This program is free software; you can redistribute it and*or modify it
** under the terms of the GNU General Public License as published by the Free
** Software Foundation; either version 2 of the License, or (at your
** option) any later version.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
** FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
** more details.
**
** You should have received a copy of the GNU General Public License along
** with this program; if not, write to the Free Software Foundation, Inc.,
** 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
**
** To contact FSC, please send an email to support@fsc.co.uk or write to
** FSC Ltd., Cardiff Business Technology Centre, Senghenydd Road, Cardiff,
** CF24 4AY.
**
******************************************************************************/

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

/** Creates a Text Field with a specified number of columns,
  * which only accepts characters not present in the exclusive string.
  * Length restricted to specified value.
  *
  * @author Ben Duncan
  */
/* Modified by R. Sparey-Taylor */


public class RestrictedLengthExclusiveField extends JTextField {

    private int _length; //holds number of characters allowed in text field
    private String _excludes; //holds the string of excluded chars

/** Creates a Text Field with a specified number of columns,
  * which only accepts characters not present in the exclusive string.
  * @param cols The number of columns in the Text Field
  */
    public RestrictedLengthExclusiveField(int cols, int length, String excludes) {
	super(new RestrictedLengthExclusiveDocument(length, excludes), null, cols);
	_excludes = excludes;
	_length = length;
    }

/** Creates a Text Field with a specified number of columns,
  * which only accepts characters not present in the exclusive string.
  */
    public RestrictedLengthExclusiveField(int length, String excludes) {
	super(new RestrictedLengthExclusiveDocument(length, excludes), null, 0);
	_excludes = excludes;
	_length =length;
    }

/** This method returns the Text from the Text Field.
  * @returns String The text from the Text Field.
  */
    	public String getTextString() {
		return(new String(super.getText()));
    	}

    protected Document createDefaultModel() {
    	return new RestrictedLengthExclusiveDocument(_length, _excludes);
    }

}
