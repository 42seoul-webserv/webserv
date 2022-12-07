#!/usr/bin/perl -w
use CGI;

$upload_dir = "../www/html/content";

$query = new CGI;

$filename = $query->param("file");
$filename =~ s/.*[\/\\](.*)/$1/;
$upload_filehandle = $query->upload("file");

open UPLOADFILE, ">$upload_dir/$filename";

while ( <$upload_filehandle> )
    {
        print UPLOADFILE;
}

close UPLOADFILE;

print "Status: 200 OK\r\n";
print $query->header ( );
print <<END_HTML;

<HTML>
<HEAD>
<TITLE>Thanks!</TITLE>
</HEAD>

<BODY>

<P>Thanks for uploading your file!</P>
<P>Your file: $upload_dir/$filename</P>

</BODY>
</HTML>

END_HTML