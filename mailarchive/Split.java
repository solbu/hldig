//
// Split.java
//
// Simple program to take a standard Berkeley mail file and create
// an HTML archive out of it.
//

import java.io.*;
import java.util.*;

public class Split
{
    public static final String		targetDirectory = "./msgs";
    public static int               messageNumber = 0;
    public static String[]          months =
    {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

    public static void main(String[] args) throws Exception
    {
	    String		    line;
	    DataInputStream	in = new DataInputStream(new BufferedInputStream(new FileInputStream(args[0])));
        Vector          message = new Vector();
	    boolean         hadBlank = true;

	    //
	    // Make sure that the directory there.
	    //
	    File            newdir = new File(targetDirectory);
	    newdir.mkdirs();

        PrintStream     index = new PrintStream(new FileOutputStream(targetDirectory + "/index.html"));

        outputHeader(index);

	    while (true)
	    {
	        line = in.readLine();
	        if (line == null)
	        {
		        break;
	        }
	        if (hadBlank && line.startsWith("From "))
	        {
	            //
	            // Start of a new message.  First process the old one.
	            //
	            if (message.size() > 0)
	            {
	                processMessage(index, message);
	                message.setSize(0);
	            }
	        }
	        if (line.length() == 0)
	        {
	            hadBlank = true;
	        }
	        else
	        {
	            hadBlank = false;
	        }
	        message.addElement(line);
	    }

	    //
	    // The final message still needs to be processed...
	    //
	    processMessage(index, message);

	    outputFooter(index);
	    index.close();
    }

    public static void outputHeader(PrintStream index) throws Exception
    {
        index.println("<html><head><title>ht://Dig mailing list archive</title></head>");
        index.println("<body bgcolor=\"#ffffff\">");
        index.println("<h1>ht://Dig mailing list archive</h1>");
        index.println("<ul>");
    }

    public static void outputFooter(PrintStream index) throws Exception
    {
        index.println("</ul>");
        index.println("</body></html>");
    }

    public static void processMessage(PrintStream index, Vector message) throws Exception
    {
        System.out.println("Got message " + messageNumber + ".  length " + message.size());

        PrintStream out = new PrintStream(new BufferedOutputStream(new FileOutputStream(targetDirectory + '/' + messageNumber + ".html")));

        //
        // Extract the header fields we are interested in
        //
        int         i;
        Hashtable   headers = new Hashtable();
        for (i = 0; i < message.size() && ((String)message.elementAt(i)).length() > 0; i++)
        {
            String  line = (String) message.elementAt(i);
            if (line.charAt(0) == ' ' || line.charAt(0) == '\t')
                continue;
            int     colon = line.indexOf(':');
            if (colon > 0)
            {
                headers.put(line.substring(0, colon), line.substring(colon + 2));
            }
        }

        out.println("<html><head><title>ht://Dig mailing list archive message " + messageNumber + "</title>");
        out.println("</head><body bgcolor=\"ffffff\">");

        out.println("<table border=0>");
        headerLine(out, "Subject", headers);
        headerLine(out, "From", headers);
        headerLine(out, "Date", headers);
        headerLine(out, "Organization", headers);
        out.println("</table>");

        out.println("<xmp>");
        for (; i < message.size(); i++)
        {
            out.println(message.elementAt(i));
        }
        out.println("</xmp></body></html>");
        out.close();

        String      subject = (String) headers.get("Subject");
        if (subject.startsWith("htdig: "))
            subject = subject.substring(7);
        else if (subject.startsWith("Re: htdig: "))
            subject = "Re: " + subject.substring(10);

        String  dateString = (String) headers.get("Date");
        String  from = (String) headers.get("From");
        if (from == null)
            from = "";
        subject = translateString(subject);
        from = translateString(from);
        index.println("<li><b><a href=" + messageNumber + ".html>" + subject + "</a></b><br>");
        index.println("<font size=-1>" + from + "</font>");
        index.println("<font size=-2>" + dateString + "</font>");
        index.println("</li>");
        messageNumber++;
    }

    public static void headerLine(PrintStream out, String key, Hashtable headers)
    {
        if (!headers.containsKey(key))
        {
            return;
        }
        out.print("<tr><td><b>" + key + ":</b></td><td>");
        out.print(translateString((String)headers.get(key)));
        out.println("</td></tr>");
    }

    public static String translateString(String s)
    {
        StringBuffer    b = new StringBuffer();
        for (int i = 0; i < s.length(); i++)
        {
            char    c = s.charAt(i);
            if (c == '<')
            {
                b.append("&lt;");
            }
            else if (c == '>')
            {
                b.append("&gt;");
            }
            else if (c == '&')
            {
                b.append("&amp;");
            }
            else
            {
                b.append(c);
            }
        }
        return b.toString();
    }
}

