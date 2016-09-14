/********* super.c code ***************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include "ext2type.h" 

typedef unsigned int   u32;

// define shorter TYPES, save typing efforts
typedef struct ext2_group_desc  GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;    // need this for new version of e2fs

GD    *gp;
SUPER *sp;
INODE *ip;
DIR   *dp; 

/******************* in <ext2fs/ext2_fs.h>*******************************
struct ext2_super_block {
  u32  s_inodes_count;       // total number of inodes
  u32  s_blocks_count;       // total number of blocks
  u32  s_r_blocks_count;     
  u32  s_free_blocks_count;  // current number of free blocks
  u32  s_free_inodes_count;  // current number of free inodes 
  u32  s_first_data_block;   // first data block in this group
  u32  s_log_block_size;     // 0 for 1KB block size
  u32  s_log_frag_size;
  u32  s_blocks_per_group;   // 8192 blocks per group 
  u32  s_frags_per_group;
  u32  s_inodes_per_group;    
  u32  s_mtime;
  u32  s_wtime;
  u16  s_mnt_count;          // number of times mounted 
  u16  s_max_mnt_count;      // mount limit
  u16  s_magic;              // 0xEF53
  // MANY MORE non-essential fields
};
**********************************************************************/

#define BLKSIZE 1024
char buf[BLKSIZE];
int fd;

int get_block(int fd, int blk, char buf[ ])
{
  lseek(fd, (long)blk*BLKSIZE, 0);
  read(fd, buf, BLKSIZE);
}

super()
{
  // read SUPER block
  get_block(fd, 1, buf);  
  sp = (SUPER *)buf;

  // check for EXT2 magic number:

  printf("s_magic = %x\n", sp->s_magic);
  if (sp->s_magic != 0xEF53){
    printf("NOT an EXT2 FS\n");
    exit(1);
  }

  printf("EXT2 FS OK\n");

  printf("s_inodes_count = %d\n", sp->s_inodes_count);
  printf("s_blocks_count = %d\n", sp->s_blocks_count);
  printf("s_free_inodes_count = %d\n", sp->s_free_inodes_count);
  printf("s_free_blocks_count = %d\n", sp->s_free_blocks_count);

  printf("s_first_data_blcok = %d\n", sp->s_first_data_block);

  printf("s_log_block_size = %d\n", sp->s_log_block_size);

  printf("s_blocks_per_group = %d\n", sp->s_blocks_per_group);
  printf("s_inodes_per_group = %d\n", sp->s_inodes_per_group);


  printf("s_mnt_count = %d\n", sp->s_mnt_count);
  printf("s_max_mnt_count = %d\n", sp->s_max_mnt_count);

  printf("s_magic = %x\n", sp->s_magic);

  printf("s_mtime = %s", ctime(&sp->s_mtime));
  printf("s_wtime = %s", ctime(&sp->s_wtime));
}
/*****************************************************************************/
gd()
{
  get_block(fd, 2, buf);
  gp = (GD *)buf;

  /****************************************************************************
  struct ext2_group_desc
{
  u32  bg_block_bitmap;          // Bmap block number
  u32  bg_inode_bitmap;          // Imap block number
  u32  bg_inode_table;           // Inodes begin block number
  u16  bg_free_blocks_count;     // THESE are OBVIOUS
  u16  bg_free_inodes_count;
  u16  bg_used_dirs_count;

  u16  bg_pad;                   // ignore these 
  u32  bg_reserved[3];
};
  ****************************************************************************/

  printf("\nGroup Descriptor stuff:\n\n");
  printf("bg_block_bitmap = %d\n", gp->bg_block_bitmap);  
  printf("bg_inode_bitmap = %d\n", gp->bg_inode_bitmap);
  printf("bg_inode_table = %d\n", gp->bg_inode_table );
  printf("bg_free_blocks_count = %d\n", gp->bg_free_blocks_count );
  printf("bg_free_inodes_count = %d\n", gp->bg_free_inodes_count );
  printf("bg_used_dirs_count = %d\n", gp->bg_used_dirs_count );
  
}

/*****************************************************************************/
int tst_bit(char *buf, int bit)
{
  int i, j;
  i = bit / 8;  j = bit % 8;
  return (buf[i] & (1 << j));
}

imap()
{
  char buf[BLKSIZE];
  int  imap, ninodes;
  int  i;

  // read SUPER block
  get_block(fd, 1, buf);
  sp = (SUPER *)buf;


  printf("\nImap Things\n");
  ninodes = sp->s_inodes_count;
  printf("ninodes = %d\n", ninodes);

  // read Group Descriptor 0
  get_block(fd, 2, buf);
  gp = (GD *)buf;

  imap = gp->bg_inode_bitmap;
  printf("imap = %d\n", imap);

  // read inode_bitmap block
  get_block(fd, imap, buf);

  for (i=0; i < ninodes; i++){
    (tst_bit(buf, i)) ?	putchar('1') : putchar('0');
    if (i && (i % 8)==0)
    {
       printf(" ");
       if (i % 32 == 0)
         printf("\n");
    }
  }
  printf("\n");
}
/*****************************************************************************/
bmap()
{
  char buf[BLKSIZE];
  int bmap, blocks;
  int i;

  // read SUPER block to get the block count yo
  get_block(fd, 1, buf);
  sp = (SUPER *)buf;

  printf("\n#justblockthings\n");
  blocks = sp->s_blocks_count;
  printf("nblocks = %d\n", blocks);

  // read Group Descriptor 0 to find the block bitmap
  get_block(fd, 2, buf);
  gp = (GD *)buf;
  
  // In this case this should be '8'
  bmap = gp->bg_block_bitmap;

  get_block(fd, bmap, buf);

  // I'll copy kc's tst_bit part found in the pre-made imap
    for (i=0; i < blocks; i++){
    (tst_bit(buf, i)) ? putchar('1') : putchar('0');
    if (i && (i % 8)==0)
    {
       printf(" ");
      if ( (i % (32) == 0))
        printf("\n");
    }
  }
  printf("\n");

}
/********* inode.c: print information in / INODE (INODE #2) *********/
int iblock;
inode()
{
  char buf[BLKSIZE];
  // read GD
  get_block(fd, 2, buf);
  gp = (GD *)buf;
  iblock = gp->bg_inode_table;   // get inode start block#
  printf("inode_block=%d\n", iblock);

  // get inode start block     
  get_block(fd, iblock, buf);

  ip = (INODE *)buf + 1;         // ip points at 2nd INODE
  printf("mode=%4x ", ip->i_mode);
  printf("uid=%d  gid=%d\n", ip->i_uid, ip->i_gid);
  printf("size=%d\n", ip->i_size);
  printf("mount time=%s",   ctime(&ip->i_mtime));
  printf("link=%d\n", ip->i_links_count);
  printf("i_block[0]=%d\n", ip->i_block[0]);

 /*****************************
  u16  i_mode;                      // fiel type|permission bits
  u16  i_uid;                       // ownerID
  u32  i_size;                      // file size in bytes
  u32  i_atime;                     // time fields  
  u32  i_ctime;
  u32  i_mtime;
  u32  i_dtime;
  u16  i_gid;                       // groupID
  u16  i_links_count;               // link count
  u32  i_blocks;                    // IGNORE
  u32  i_flags;                     // IGNORE
  u32  i_reserved1;                 // IGNORE
  u32  i_block[15];                 // IMPORTANT, but later
 ***************************/
}

/*****************************************************************************/
dirc()
{
  DIR dp;
  char dbuf[BLKSIZE];
  
  // The second INODE will yield the '/' directory
  
}

/*****************************************************************************/
char *disk = "mydisk";
main(int argc, char *argv[ ])

{ 
  if (argc > 1)
    disk = argv[1];
  fd = open(disk, O_RDONLY);
  if (fd < 0){
    printf("open failed\n");
    exit(1);
  }

  super();
  gd();
  imap();
  bmap();
  inode();
}

