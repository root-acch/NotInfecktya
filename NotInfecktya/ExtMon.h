#include <ntdef.h>

const WCHAR* monitoredExtensions[] = {
    L".r5a",L".micro",L".pwned",L".paymeme",L".omfl",L".globe",L".onion",L".badnews",L".thanos",
    L".cerber",L".zepto",L".nemesis",L".wallet",L".mole",L".karma",L".java",L".power",L".crinf",
    L".maktub",L".nozelesn",L".petya",L".raid",L".aesir",L".hslock",L".xdata",L".vape",L".encrypTile",
    L".cerber2",L".lechiffre",L".prosper",L".thanos",L".edcry1",L".bloccato",L".crypted",L".lukitus",
    L".unlock92",L".crypte",L".encryptedRSA",L".venom",L".datakiller",L".gandcrab",L".kraken",L".vault",
    L".hnumkhotep",L".vesad",L".actin",L".matrix",L".btc",L".etols",L".dharma",L".hese",L".kuus",L".pumas",
    L".apocalypse",L".revenge",L".zzzzzz",L".trump",L".nazar",L".pizdec",L".cobra",L".WCRY",L".WNCRY", L".wcry",L".wncry",L".thebad@cock.li",
    L".payransom",L".porno",L".btc.gws",L".kkk",L".fun",L".locky",L".aesir",L".osiris",L".GDCB",L".CRAB",L".KRAB",L".R5A",
    L".KRAB",L".ecc",L".ezz",L".exx",L".xyz",L".zzz",L".aaa",L".abc",L".ccc",L".matrix",L".dharma",L".wallet",L".phobos",
    L".phoenix",L".decrypt2019",L".crimson",L".maze",L".mz",L".mzmx",L".mzzq",L".mzzz",L".egregor", L".lock", L".gpr", L".lock~",
    L".rep", L".abfa", L".encrypted", L".FuckYourData", L".locked", L".Encryptedfile", L".SecureCrypted", L".johnycryptor@hackermail.com.xtbl",
    L".ecovector2@aol.com.xtbl", L".ACRYPT", L".frozen", L".doomed", L".fun", L".szf"
};

const WCHAR* EntropyExtensions[] = {
    L".zip", L".odt", L".odp", L".ott", L".oxps",
    L".docx", L".pptx", L".xlsx", L".xls", L".jpe", L".jpeg",
    L".jpg", L".png", L".com", L".rar", 
    L".html", L".csv", L".pdf"
};

UCHAR magicBytes1[5][4] = {
    {0x25, 0x50, 0x44, 0x46},
    {0x50, 0x4B, 0x03, 0x04},
    {0x00, 0x00, 0x03, 0xF3},
    {0xFF, 0xD8, 0xFF, 0xE1},
    {0xFF, 0xD8, 0xFF, 0xE0},
};

UCHAR magicBytes2[4][8] = {
    {0x50, 0x4B, 0x03, 0x04, 0x14, 0x00, 0x06, 0x00},
    {0x09, 0x08, 0x10, 0x00, 0x00, 0x06, 0x05, 0x00},
    {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A},
    {0x52, 0x61, 0x72, 0x21, 0x1A, 0x07, 0x01, 0x00}
};

UCHAR magicBytes3[1][2] = {
    {0xFF, 0xD8},
};

UCHAR magicBytes5[1][10] = {
     {0x3C, 0x21, 0x44, 0x4F, 0x43, 0x54, 0x59, 0x50, 0x45, 0x20}
};

UCHAR magicBytes6[1][7] = {
     {0x52, 0x61, 0x72, 0x21, 0x1A, 0x07, 0x00}
};

UCHAR magicBytes7[1][12] = {
     {0x48, 0x61, 0x72, 0x6D, 0x6F, 0x6E, 0x79, 0x43, 0x53, 0x56, 0x20, 0x76}
};