// SPDX-License-Identifier: GPL-2.0-or-later
package com.vrlightgunhub.vcop;
import java.io.*;
import java.security.*;
import java.util.*;
import java.util.zip.*;

/** Validate original packed ROMs in place; never unpack game data into app state. */
public final class GameFiles {
    static final class Rom {
        final String name, sha1; final long size, crc;
        Rom(String name, long size, long crc, String sha1) {
            this.name=name; this.size=size; this.crc=crc; this.sha1=sha1;
        }
    }
    private static final Rom[] REQUIRED = {
        new Rom("epr-17166b.12", 131072, 0xa5647c59L, "0a9e0be447d3591e82efd40ef4acbfe7ae211579"),
        new Rom("epr-17167b.13", 131072, 0xf5dde26aL, "95db029bc4206a44ea216afbcd1c19689f79115a"),
        new Rom("epr-17160a.14", 131072, 0x267f3242L, "40ec09cda984bb80969bfae2278432153137c213"),
        new Rom("epr-17161a.15", 131072, 0xf7126876L, "b0ceb1206edaa507ec15723497fcd447a511f423"),
        new Rom("mpr-17164.10", 2097152, 0xac5fc501L, "e60deec1e79d207d37d3f4ddd83a1b2125c411ac"),
        new Rom("mpr-17165.11", 2097152, 0x82296d00L, "23327137b36c98dfb9175ea9d36478e7385dfac2"),
        new Rom("mpr-17162.8", 2097152, 0x60ddd41eL, "0894c9bcdedeb09f921419a309858e242cb8db3a"),
        new Rom("mpr-17163.9", 2097152, 0x8c1f9dc8L, "cf99a5bb4f343d59c8d6f5716287b6e16bef6412"),
        new Rom("epr-17168a.6", 524288, 0x59091a37L, "14591c7015aaf126755be584aa94c04e6de222fa"),
        new Rom("epr-17169a.7", 524288, 0x0495808dL, "5b86a9a68c2b52f942aa8d858ee7a491f546a921"),
        new Rom("mpr-17159.16", 2097152, 0xe218727dL, "1458d01d49936a0b8d497b62ff9ea940ca753b37"),
        new Rom("mpr-17156.20", 2097152, 0xc4f4aabfL, "8814cd329609cc8a188fedd770230bb9a5d00361"),
        new Rom("mpr-17158.25", 2097152, 0x1108d1ecL, "e95d4166bd4b26c5f21b85821b410f53045f4309"),
        new Rom("mpr-17157.24", 2097152, 0xcf31e33dL, "0cb62d4f28b5ad8a7e4c82b0ca8aea3037b05455"),
        new Rom("epr-17170.7", 131072, 0x06a38ae2L, "a2c3d14d9266449ebfc6d976a956e0a8a602cfb0"),
        new Rom("epr-17171.8", 131072, 0xb5e436f8L, "1da3cb52d64f52d03a8de9954afffbc6e1549a5b"),
        new Rom("mpr-17172.32", 1048576, 0xab22cac3L, "0e872158faeb8c0404b10cdf0a3fa36f89a5093e"),
        new Rom("mpr-17173.33", 1048576, 0x3cb4005cL, "a56f436ea6dfe0968b73ae7bc92bb2f4c612460d"),
        new Rom("mpr-17174.4", 2097152, 0xa50369ccL, "69807157baf6e3679adc95633c82b0236db01247"),
        new Rom("mpr-17175.5", 2097152, 0x9136d43cL, "741f80a8ff8165ffe171dc568e0da4ad0bde4809"),
        new Rom("opr-14742a.45", 131072, 0x90c6b117L, "f46429fffcee17d056f56d5fe035a33f1fd6c27e"),
        new Rom("opr-14743a.46", 131072, 0xae7f446bL, "5b9f1fc47caf21e061e930c0d72804e4ec8c7bca"),
        new Rom("opr-14744.58", 131072, 0x730ea9e0L, "651f1db4089a400d073b19ada299b4b08b08f372"),
        new Rom("opr-14745.59", 131072, 0x4c934d96L, "e3349ece0e47f684d61ad11bfea4a90602287350"),
        new Rom("opr-14746.62", 131072, 0x2a266cbdL, "34e047a93459406c22acf4c25089d1a4955f94ca"),
        new Rom("opr-14747.63", 131072, 0xa4ad5e19L, "7d7ec300eeb9a8de1590011e37108688c092f329"),
        new Rom("epr-17181.6", 65536, 0x1add2b82L, "81892251d466f630a96af25bde652c20e47d7ede"),
        new Rom("hd44780_a00.bin", 4096, 0xe459877cL, "65cf075a988cdcbb316b9afdd0529b374a1a65ec"),
    };
    private static final long MAX_ARCHIVE_BYTES=128L*1024*1024;
    private static final int MAX_MEMBERS=4096;
    private GameFiles() {}

    public static void validate(File game, File... supplemental) throws IOException {
        if(game==null || !game.isFile()) throw new IOException("Missing Download/vcop-vr/vcop.zip. Copy your owned original arcade ROM archive to Download.");
        List<File> paths=new ArrayList<>(); paths.add(game);
        for(File file:supplemental) if(file!=null && file.isFile()) paths.add(file);
        validateArchives(paths.toArray(new File[0]), REQUIRED);
    }

    static void validateArchives(File[] paths, Rom[] requirements) throws IOException {
        Map<String,Rom> wanted=new HashMap<>();
        for(Rom rom:requirements) wanted.put(rom.name,rom);
        Set<String> found=new HashSet<>();
        for(File path:paths) {
            if(!path.isFile() || !path.canRead()) throw new IOException("Cannot read "+path.getName()+". Allow game file access and retry.");
            if(path.length()>MAX_ARCHIVE_BYTES) throw new IOException("ROM archive is too large: "+path.getName());
            try(ZipFile zip=new ZipFile(path)) {
                if(zip.size()>MAX_MEMBERS) throw new IOException("Too many ZIP members in "+path.getName());
                Set<String> seen=new HashSet<>();
                Enumeration<? extends ZipEntry> entries=zip.entries();
                while(entries.hasMoreElements()) {
                    ZipEntry entry=entries.nextElement(); String name=entry.getName();
                    if(name.isEmpty() || name.equals(".") || name.equals("..") || name.indexOf('/')>=0 || name.indexOf('\\')>=0 || name.indexOf(':')>=0 || name.indexOf('\0')>=0)
                        throw new IOException("Unsafe ZIP member: "+name);
                    String key=name.toLowerCase(Locale.ROOT);
                    if(!seen.add(key)) throw new IOException("Duplicate ZIP member: "+name);
                    Rom rom=wanted.get(key);
                    if(rom==null) continue;
                    if(entry.isDirectory() || entry.getSize()!=rom.size) throw new IOException("Wrong ROM size: "+rom.name);
                    MessageDigest sha;
                    try { sha=MessageDigest.getInstance("SHA-1"); }
                    catch(NoSuchAlgorithmException error) { throw new IOException("SHA-1 validation is unavailable",error); }
                    CRC32 crc=new CRC32(); byte[] buffer=new byte[16384]; long count=0;
                    try(InputStream in=zip.getInputStream(entry)) {
                        int bytes;
                        while((bytes=in.read(buffer,0,(int)Math.min(buffer.length,rom.size-count+1)))!=-1) {
                            if(bytes==0) continue;
                            count+=bytes;
                            if(count>rom.size) throw new IOException("Oversized ROM payload: "+rom.name);
                            sha.update(buffer,0,bytes); crc.update(buffer,0,bytes);
                        }
                    }
                    if(count!=rom.size || crc.getValue()!=rom.crc || !hex(sha.digest()).equals(rom.sha1)) throw new IOException("ROM checksum mismatch: "+rom.name+". Copy a complete, unmodified archive.");
                    found.add(key);
                }
            } catch(ZipException error) {
                throw new IOException("Cannot read "+path.getName()+": invalid, corrupt or encrypted ZIP ("+error.getMessage()+").",error);
            }
        }
        for(Rom rom:requirements) if(!found.contains(rom.name)) {
            String source=(rom.name.equals("epr-17181.6")||rom.name.equals("hd44780_a00.bin"))?"Download/vcop-vr/model1io2.zip (I/O/LCD firmware)":"Download/vcop-vr/vcop.zip";
            throw new IOException("Missing ROM: "+rom.name+". Required in "+source+" or a complete vcop.zip.");
        }
    }
    private static String hex(byte[] bytes) {
        char[] symbols="0123456789abcdef".toCharArray(); char[] result=new char[bytes.length*2];
        for(int i=0;i<bytes.length;i++) { result[i*2]=symbols[(bytes[i]&255)>>>4]; result[i*2+1]=symbols[bytes[i]&15]; }
        return new String(result);
    }
}
