
#!/usr/bin/perl -w
#
# bbdiff - Command-line tool to invoke BBEdit's Find Differences command on Mac OS X.
#   by John Gruber
#   http://daringfireball.net
#
# Version 1.1
# Wed Dec 4, 2002
#
# See POD for details (type "perldoc bbdiff").
#

use strict;
use Getopt::Std;
use File::Spec::Unix;


our (%opts);        # hash to store command-line switches
getopts( 'bis', \%opts);

# Set these options to string values, because they're used in the AppleScript:
my $background        = "false";
my $case_sensitive    = "true";
my $ignore_whitespace = "false";
if ($opts{'b'}) { $background        = "true"; }
if ($opts{'i'}) { $case_sensitive    = "false"; }
if ($opts{'s'}) { $ignore_whitespace = "true"; }

# Make sure there are two, and only two, file arguments.
my $count = @ARGV;
if ($count != 2) {
    die usage();
}

# Get full paths to file arguments.
my ($old_file, $new_file) = @ARGV;
$new_file = File::Spec::Unix->rel2abs( $new_file );
$old_file = File::Spec::Unix->rel2abs( $old_file );

# Make sure each argument is an existing plain file.
foreach ($old_file, $new_file) {
    ( -f ) or die $_ . " is not a file.\n";
}

# NOTE: You must change the name of the BBEdit application file in the
# 'tell application' line below to match the name of your BBEdit
# application file. For example, if your copy of BBEdit is named "BBEdit
# 6.5", you'll need to change the tell statement to 'tell application
# "BBEdit 6.5"'. Alternatively, you can simply rename your copy of BBEdit
# to "BBEdit".

my $comp_script = qq[
tell application "BBEdit"
    set comp_opts to {case sensitive:$case_sensitive, ignore leading spaces:$ignore_whitespace, ignore trailing spaces:$ignore_whitespace}
    set comp_result to compare POSIX file "$new_file" against POSIX file "$old_file" options comp_opts
    if differences found of comp_result then
        if not $background then activate
        return "Differences found."
    else
        set the_reason to reason for no differences of comp_result
        return "No differences found: " & the_reason
    end if
end tell
];

system ( "/usr/bin/osascript -e '$comp_script'" );


sub usage {
    die << "ENDUSAGE";
Usage: $0 [-b -i -s] oldfile newfile
    -b keep BBEdit in the background
    -i case insensitive comparison
    -s ignore leading and trailing whitespace on lines

ENDUSAGE
}

__END__


=pod

=head1 NAME

B<bbdiff> - Command-line tool to invoke BBEdit's Find Differences command on Mac OS X.


=head1 SYNOPSIS

    bbdiff [-b -i -s] oldfile newfile


=head1 OPTIONS

B<bbdiff> accepts these options:

=over 4

=item B<-b>

Keep BBEdit in the background. By default, BBEdit will activate if
differences are found.

=item B<-i>

Case insensitive comparison.

=item B<-s>

Ignore leading and trailing whitespace on lines.

=back


=head1 VERSION HISTORY

    1.0: Sat Nov 30, 2002
    Initial release.

    1.1: Wed Dec 4, 2002
    Swapped order of oldfile and newfile in argument list, to match usage
        of existing diff tools.
    Convert arguments to full paths before testing whether the files exist.


=head1 AUTHOR

    John Gruber
    http://daringfireball.net/projects/bbdiff/


=head1 COPYRIGHT and LICENSE

Copyright (c) 2002 John Gruber.

This program is free and open software. You may use, copy, modify, distribute,
and sell this program (and any modified variants) in any way you wish,
provided you do not restrict others from doing the same.

=cut -it

{
  "configurations": [
    {
      "type": "cppdbg",
      "request": "launch",
      "name": "Launch Program",
      "program": "${workspaceFolder}/${input:executablePath}",
      "stopAtEntry": true
    }
  ],
  "inputs": [
    {
      "type": "promptString",
      "id": "executablePath",
      "description": "Path to your