#!/usr/bin/env ruby

require "fileutils"
require "zip/zip"
require "zip/zipfilesystem"

FileUtils.rm_f( "resources.dat" )
FileUtils.rm_f( "world.db3" )
FileUtils.cp( "Resources/world.db3", "." )
Zip::ZipFile.open( "resources.dat", Zip::ZipFile::CREATE) do |zipfile|
    Dir.entries( "Resources" ).each do |filename|
        next unless filename =~ /\.(png|mod|res|fnt|psi|wav)$/
        zipfile.add( filename, File.join( "Resources", filename ) )
    end
end

FileUtils.rm_f( "Release/resources.dat" )
FileUtils.rm_f( "Release/world.db3" )
FileUtils.cp( "resources.dat", "Release" )
FileUtils.cp( "world.db3", "Release" )
