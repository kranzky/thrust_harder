#!/usr/bin/env ruby

require "fileutils"
require "zip/zip"
require "zip/zipfilesystem"

FileUtils.rm_f( "release.zip" )
Zip::ZipFile.open( "release.zip", Zip::ZipFile::CREATE) do |zipfile|
    zipfile.add( "world.db3", "world.db3" )
    zipfile.add( "resources.dat", "resources.dat" )
    zipfile.add( "ThrustHarder.exe",
                 File.join( "Release", "ThrustHarder.exe" ) )
    zipfile.add( "hge.dll",
                 File.join( "..", "..", "ThirdParty", "hge", "hge.dll" ) )
    zipfile.add( "bass.dll",
                 File.join( "..", "..", "ThirdParty", "hge", "bass.dll" ) )
    zipfile.add( "sqlite3.dll",
                File.join( "..", "..", "ThirdParty", "sqlite", "sqlite3.dll" ) )
end
