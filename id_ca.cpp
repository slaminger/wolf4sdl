// ID_CA.C

// this has been customized for WOLF

/*
=============================================================================

Id Software Caching Manager
---------------------------

Must be started BEFORE the memory manager, because it needs to get the headers
loaded into the data segment

=============================================================================
*/
/*
#include <sys/types.h>
#if defined _WIN32
	#include <io.h>
#elif !defined _arch_dreamcast
	#include <sys/uio.h>
	#include <unistd.h>
#endif
 */
#include "wl_def.h"
#pragma hdrstop
#define THREEBYTEGRSTARTS

/*
=============================================================================

                             LOCAL CONSTANTS

=============================================================================
*/

typedef struct
{
    word bit0,bit1;       // 0-255 is a character, > is a pointer to a node
} huffnode;


typedef struct
{
    word RLEWtag;
    int32_t headeroffsets[100];
} mapfiletype;


/*
=============================================================================

                             GLOBAL VARIABLES

=============================================================================
*/

#define BUFFERSIZE 0x1000
static int32_t bufferseg[BUFFERSIZE/4];

int     mapon;

word    *mapsegs[MAPPLANES];
static maptype* mapheaderseg[NUMMAPS];
byte    *audiosegs[NUMSNDCHUNKS];
byte    *grsegs[NUMCHUNKS];

word    RLEWtag;

int     numEpisodesMissing = 0;

/*
=============================================================================

                             LOCAL VARIABLES

=============================================================================
*/

char extension[5]; // Need a string, not constant to change cache files
char graphext[5];
char audioext[5];
static const char gheadname[] = "vgahead.";
static const char gfilename[] = "vgagraph.";
static const char gdictname[] = "vgadict.";
static const char mheadname[] = "maphead.";
static const char mfilename[] = "maptemp.";
static const char aheadname[] = "audiohed.";
static const char afilename[] = "audiot.";

void CA_CannotOpen(const char *string);

static int32_t  grstarts[NUMCHUNKS + 1];
static int32_t* audiostarts; // array of offsets in audio / audiot

#ifdef GRHEADERLINKED
huffnode *grhuffman;
#else
huffnode grhuffman[255];
#endif

int    grhandle = -1;               // handle to EGAGRAPH
int    maphandle = -1;              // handle to MAPTEMP / GAMEMAPS
int    audiohandle = -1;            // handle to AUDIOT / AUDIO

int32_t   chunkcomplen,chunkexplen;

SDMode oldsoundmode;


static int32_t GRFILEPOS(const size_t idx)
{
	assert(idx < lengthof(grstarts));
	return grstarts[idx];
}

/*
=============================================================================

                            LOW LEVEL ROUTINES

=============================================================================
*/

/*
============================
=
= CAL_GetGrChunkLength
=
= Gets the length of an explicit length chunk (not tiles)
= The file pointer is positioned so the compressed data can be read in next.
=
============================
*/

void CAL_GetGrChunkLength (int chunk)
{
    //lseek(grhandle,GRFILEPOS(chunk),SEEK_SET);
    //read(grhandle,&chunkexplen,sizeof(chunkexplen));

	//GFS_Load(grhandle, 0, (void *)&chunkexplen, sizeof(chunkexplen));


	uint8_t *Chunks;
	uint16_t delta;
	uint32_t delta2;
	Uint16 i,j=0;
	delta = (uint16_t)(GRFILEPOS(chunk)/2048);
	delta2 = (GRFILEPOS(chunk) - delta*2048); 
	Chunks=(uint8_t*)malloc(8192+sizeof(chunkexplen));
	//slPrint("                                    ",slLocate(2,17));
	//slPrint("CAL_GetGrChunkLength",slLocate(2,17));
	CHECKMALLOCRESULT(Chunks);
	GFS_Load(grhandle, delta, (void *)Chunks, sizeof(chunkexplen));

	memcpy(&chunkexplen,&Chunks[delta2],sizeof(chunkexplen));
	free(Chunks);
  
	chunkexplen = SWAP_BYTES_32(chunkexplen);
    chunkcomplen = GRFILEPOS(chunk+1)-GRFILEPOS(chunk)-4;
	// VBT correct
}

/*
============================================================================

                COMPRESSION routines, see JHUFF.C for more

============================================================================
*/

static void CAL_HuffExpand(byte *source, byte *dest, int32_t length, huffnode *hufftable)
{
    byte *end;
    huffnode *headptr, *huffptr;

    if(!length || !dest)
    {
        Quit("length or dest is null!");
        return;
    }
	//slPrint("                                    ",slLocate(2,17));
	//slPrint("CAL_HuffExpand",slLocate(2,17));
    headptr = hufftable+254;        // head node is always node 254

    int written = 0;

    end=dest+length;

    byte val = *source++;
    byte mask = 1;
    word nodeval;
    huffptr = headptr;
    while(1)
    {
        if(!(val & mask))
            nodeval = huffptr->bit0;
        else
            nodeval = huffptr->bit1;
        if(mask==0x80)
        {
            val = *source++;
            mask = 1;
        }
        else mask <<= 1;

        if(nodeval<256)
        {
            *dest++ = (byte) nodeval;
            written++;
            huffptr = headptr;
            if(dest>=end) break;
        }
        else
        {
            huffptr = hufftable + (nodeval - 256);
        }
    }
}

/*
======================
=
= CAL_CarmackExpand
=
= Length is the length of the EXPANDED data
=
======================
*/

#define NEARTAG 0xa7
#define FARTAG  0xa8

void CAL_CarmackExpand (byte *source, word *dest, int length)
{
    word ch,chhigh,count,offset;
    byte *inptr;
    word *copyptr, *outptr;
	//slPrint("                                    ",slLocate(2,17));
	//slPrint("CAL_CarmackExpand",slLocate(2,17));
    length/=2;

    inptr = (byte *) source;
    outptr = dest;
//ch |= *inptr++;"     -> "ch |= (*inptr++) << 8;
    while (length>0)
    {
        ch = READWORD(inptr);
        chhigh = ch>>8;
        if (chhigh == NEARTAG)
        {
            count = ch&0xff;
            if (!count)
            {                               // have to insert a word containing the tag byte
                ch |= *inptr++;
                *outptr = ch;
				outptr++;
                length--;
            }
            else
            {
                offset = *inptr++;
                copyptr = outptr - offset;
                length -= count;

                if(length<0) return;
                while (count--)
				{
                    *outptr = *copyptr;
					outptr++;
					copyptr++;
				}
            }
        }
        else if (chhigh == FARTAG)
        {
            count = ch&0xff;
            if (!count)
            {                               // have to insert a word containing the tag byte
                 ch |= *inptr++;
                *outptr = ch;
				outptr++;
                length --;
            }
            else
            {
                offset = READWORD(inptr);
                copyptr = dest + offset;
                length -= count;
                if(length<0) return;
                while (count--)
				{
                    *outptr = *copyptr;
					outptr++;
					copyptr++;
				}
            }
        }
        else
        {
            *outptr = ch;
			outptr++;
            length --;
        }
    }
}

/*
======================
=
= CA_RLEWcompress
=
======================
*/

int32_t CA_RLEWCompress (word *source, int32_t length, word *dest, word rlewtag)
{
    word value,count;
    unsigned i;
    word *start,*end;

	//slPrint("                                    ",slLocate(2,17));
	//slPrint("CA_RLEWCompress",slLocate(2,17));

    start = dest;

    end = source + (length+1)/2;

    //
    // compress it
    //
    do
    {
        count = 1;
        value = *source++;
        while (*source == value && source<end)
        {
            count++;
            source++;
        }
        if (count>3 || value == rlewtag)
        {
            //
            // send a tag / count / value string
            //
            *dest++ = rlewtag;
            *dest++ = count;
            *dest++ = value;
        }
        else
        {
            //
            // send word without compressing
            //
            for (i=1;i<=count;i++)
                *dest++ = value;
        }

    } while (source<end);

    return (int32_t)(2*(dest-start));
}


/*
======================
=
= CA_RLEWexpand
= length is EXPANDED length
=
======================
*/

void CA_RLEWexpand (word *source, word *dest, int32_t length, word rlewtag)
{
    word value,count,i;
    word *end=dest+length/2;

	//slPrint("                                    ",slLocate(2,17));
	//slPrint("CA_RLEWexpand",slLocate(2,17));

//
// expand it
//
    do
    {
        value = *source++;
        if (value != rlewtag)
            //
            // uncompressed
            //
            *dest++=value;
        else
        {
            //
            // compressed string
            //
            count = *source++;
            value = *source++;
            for (i=1;i<=count;i++)
                *dest++ = value;
        }
    } while (dest<end);
}



/*
=============================================================================

                                         CACHE MANAGER ROUTINES

=============================================================================
*/


/*
======================
=
= CAL_SetupGrFile
=
======================
*/

void CAL_SetupGrFile (void)
{
    char fname[13];
    //int handle;
	unsigned int i=0;
    byte *compseg;
	long fileSize;
	Sint32 fileId;

#ifdef GRHEADERLINKED

    grhuffman = (huffnode *)&EGAdict;
    grstarts = (int32_t _seg *)FP_SEG(&EGAhead);

#else

//
// load ???dict.ext (huffman dictionary for graphics files)
//
	//slPrint("                                    ",slLocate(2,17));
	//slPrint("CAL_SetupGrFile",slLocate(2,17));

    strcpy(fname,gdictname);
    strcat(fname,graphext);

	while (fname[i])
	{
		fname[i]= toupper(fname[i]);
		i++;
	}
	i=0;

    //handle = open(fname, O_RDONLY | O_BINARY);
    //if (handle == -1)
	if(stat(fname, NULL))
        CA_CannotOpen(fname);

	fileId = GFS_NameToId((Sint8*)fname);
	fileSize = GetFileSize(fileId);

	Uint8 *vbtHuff;
	vbtHuff=(Uint8 *)malloc(sizeof(grhuffman));
	CHECKMALLOCRESULT(vbtHuff);

	GFS_Load(fileId, 0, (void *)vbtHuff, sizeof(grhuffman));

	for(unsigned int x=0;x<255*4;x+=4)
	{
		grhuffman[i].bit0=vbtHuff[x] | (vbtHuff[x+1]<<8);
		grhuffman[i].bit1=vbtHuff[x+2] | (vbtHuff[x+3]<<8);
		i++;
	}	 /*
		slPrintHex(grhuffman[0].bit0,slLocate(10,10));
		slPrintHex(grhuffman[0].bit1,slLocate(10,11));		

		slPrintHex(grhuffman[254].bit0,slLocate(10,14));
		slPrintHex(grhuffman[254].bit1,slLocate(10,15));
					*/

	// VBT correct
	free (vbtHuff);
	i=0;

    //read(handle, grhuffman, sizeof(grhuffman));
    //close(handle);

    // load the data offsets from ???head.ext
    strcpy(fname,gheadname);
    strcat(fname,graphext);

	while (fname[i])
	{
		fname[i]= toupper(fname[i]);
		i++;
	}
	i=0;

    //handle = open(fname, O_RDONLY | O_BINARY);
    //if (handle == -1)
	if(stat(fname, NULL))
        CA_CannotOpen(fname);

	fileId = GFS_NameToId((Sint8*)fname);
	fileSize = GetFileSize(fileId);
    long headersize = fileSize;//lseek(handle, 0, SEEK_END);
    //lseek(handle, 0, SEEK_SET);

    if(!param_ignorenumchunks && headersize / 3 != (long) (lengthof(grstarts) - numEpisodesMissing))
        Quit("Wolf4SDL was not compiled for these data files:\n"
            "        %s contains a wrong number of offsets                          (%i instead of %i)!\n\n"
            "                          Please check whether you are using the right executable!\n"
            "(For mod developers: perhaps you forgot to update NUMCHUNKS?)",
            fname, headersize / 3, lengthof(grstarts) - numEpisodesMissing);

    byte data[lengthof(grstarts) * 3];
	//GFS_Load(fileId, 0, (void *)data, fileSize);
	GFS_Load(fileId, 0, (void *)data, sizeof(data));
    //read(handle, data, sizeof(data));
    //close(handle);

    const byte* d = data;
    for (int32_t* i = grstarts; i != endof(grstarts); ++i)
    {
        const int32_t val = d[0] | d[1] << 8 | d[2] << 16;
        *i = (val == 0x00FFFFFF ? -1 : val);
        d += 3;
    }

#endif

//
// Open the graphics file, leaving it open until the game is finished
//
    strcpy(fname,gfilename);
    strcat(fname,graphext);

	while (fname[i])
	{
		fname[i]= toupper(fname[i]);
		i++;
	}
	i=0;

    //grhandle = open(fname, O_RDONLY | O_BINARY);
    //if (grhandle == -1)
	if(stat(fname, NULL))
        CA_CannotOpen(fname);

	grhandle = GFS_NameToId((Sint8*)fname);
	fileSize = GetFileSize(grhandle);
//
// load the pic and sprite headers into the arrays in the data segment
//
    pictable=(pictabletype *) malloc(NUMPICS*sizeof(pictabletype));
    CHECKMALLOCRESULT(pictable);
    CAL_GetGrChunkLength(STRUCTPIC);                // position file pointer
	compseg =(byte*)malloc((chunkcomplen+4));
	CHECKMALLOCRESULT(compseg);
	GFS_Load(grhandle, 0, (void *)compseg, (chunkcomplen+4));
    CAL_HuffExpand(&compseg[4], (byte*)pictable, NUMPICS * sizeof(pictabletype), grhuffman);

	for(long xxx=0;xxx<NUMPICS;xxx++)
	{
		pictable[xxx].height=SWAP_BYTES_16(pictable[xxx].height);
		pictable[xxx].width=SWAP_BYTES_16(pictable[xxx].width);
	}

	free(compseg);
	// VBT correct
}

//==========================================================================


/*
======================
=
= CAL_SetupMapFile
=
======================
*/

void CAL_SetupMapFile (void)
{
    int     i,j;
    int handle;
    int32_t length,pos;
    char fname[13];
	long fileSize;

	//slPrint("                                    ",slLocate(2,17));
	//slPrint("CAL_SetupMapFile",slLocate(2,17));

//
// load maphead.ext (offsets and tileinfo for map file)
//
    strcpy(fname,mheadname);
    strcat(fname,extension);

	Sint32 fileId;
	i=0;
	while (fname[i])
	{
		fname[i]= toupper(fname[i]);
		i++;
	}	 
	i=0;
	fileId = GFS_NameToId((Sint8*)fname);
	fileSize = GetFileSize(fileId);

    //handle = open(fname, O_RDONLY | O_BINARY);
    if(stat(fname, NULL))
        CA_CannotOpen(fname);
    length = NUMMAPS*4+2; // used to be "filelength(handle);"
    mapfiletype *tinf=(mapfiletype *) malloc(sizeof(mapfiletype));
    CHECKMALLOCRESULT(tinf);
	GFS_Load(fileId, 0, (void *)tinf, length);
    //read(handle, tinf, length);
    //close(handle);
     word RLEWtag;
    int32_t headeroffsets[100];

   tinf->RLEWtag=SWAP_BYTES_16(tinf->RLEWtag);

//while(1);

   for(i=0;i<100;i++)
		tinf->headeroffsets[i]=SWAP_BYTES_32(tinf->headeroffsets[i]);

    RLEWtag=tinf->RLEWtag;
	i=0;
//
// open the data file
//
#ifdef CARMACIZED
    strcpy(fname, "gamemaps.");
    strcat(fname, extension);

	while (fname[i])
	{
		fname[i]= toupper(fname[i]);
		i++;
	}	 

	i=0;
	maphandle = GFS_NameToId((Sint8*)fname);
	fileSize = GetFileSize(maphandle);

    //maphandle = open(fname, O_RDONLY | O_BINARY);
    //if (maphandle == -1)
	if(stat(fname, NULL))
        CA_CannotOpen(fname);
#else
    strcpy(fname,mfilename);
    strcat(fname,extension);

    maphandle = open(fname, O_RDONLY | O_BINARY);
    if (maphandle == -1)
        CA_CannotOpen(fname);
#endif
//
// load all map header
//
	Uint8 *vbt;
	vbt = (Uint8*)malloc(fileSize);
	CHECKMALLOCRESULT(vbt);
	GFS_Load(maphandle, 0, (void *)vbt, fileSize);

    for (i=0;i<NUMMAPS;i++)
    {
        pos = tinf->headeroffsets[i];
        if (pos<0)                          // $FFFFFFFF start is a sparse map
            continue;	   

        mapheaderseg[i]=(maptype *) malloc(sizeof(maptype));
        CHECKMALLOCRESULT(mapheaderseg[i]);
        //lseek(maphandle,pos,SEEK_SET);
        //read (maphandle,(memptr)mapheaderseg[i],sizeof(maptype));
		memcpy((memptr)mapheaderseg[i],&vbt[pos],sizeof(maptype));
		  
	   for(j=0;j<3;j++)
		{
//vbtvbt
			mapheaderseg[i]->planestart[j]=SWAP_BYTES_32(mapheaderseg[i]->planestart[j]);
			mapheaderseg[i]->planelength[j]=SWAP_BYTES_16(mapheaderseg[i]->planelength[j]);
		}	 
		mapheaderseg[i]->width=SWAP_BYTES_16(mapheaderseg[i]->width);
		mapheaderseg[i]->height=SWAP_BYTES_16(mapheaderseg[i]->height);
    }
	free(vbt);
    free(tinf);

//
// allocate space for 3 64*64 planes
//
    for (i=0;i<MAPPLANES;i++)
    {
        mapsegs[i]=(word *) malloc(maparea*2);
        CHECKMALLOCRESULT(mapsegs[i]);
    }
}


//==========================================================================


/*
======================
=
= CAL_SetupAudioFile
=
======================
*/

void CAL_SetupAudioFile (void)
{
//#ifdef VBT
	char fname[13];
	Sint32 fileId;
	long fileSize;
	unsigned int i=0;

	//slPrint("                                    ",slLocate(2,17));
	//slPrint("CAL_SetupMapFile",slLocate(2,17));

//
// load audiohed.ext (offsets for audio file)
//
    strcpy(fname,aheadname);
    strcat(fname,audioext);

	while (fname[i])
	{
		fname[i]= toupper(fname[i]);
		i++;
	}	 
	i=0;
	fileId = GFS_NameToId((Sint8*)fname);
	fileSize = GetFileSize(fileId);

    //handle = open(fname, O_RDONLY | O_BINARY);

    int32_t* ptr;
    //if (!CA_LoadFile(fname, &ptr))
    if(stat(fname, NULL))
        CA_CannotOpen(fname);
	ptr=(int32_t*)malloc(fileSize);
	GFS_Load(fileId, 0, (void *)ptr, fileSize);

	for(i=0;i<fileSize/4;i++)
	{
		ptr[i]=SWAP_BYTES_32(ptr[i]);
	}
    audiostarts = (int32_t*)ptr;
//
// open the data file
//
    strcpy(fname,afilename);
    strcat(fname,audioext);

	while (fname[i])
	{
		fname[i]= toupper(fname[i]);
		i++;
	}	 
	i=0;

	audiohandle = GFS_NameToId((Sint8*)fname);
    //audiohandle = open(fname, O_RDONLY | O_BINARY);
    //if (audiohandle == -1)
	if(stat(fname, NULL))
        CA_CannotOpen(fname);
}

//==========================================================================


/*
======================
=
= CA_Startup
=
= Open all files and load in headers
=
======================
*/

void CA_Startup (void)
{
#ifdef PROFILE
    unlink ("PROFILE.TXT");
    profilehandle = open("PROFILE.TXT", O_CREAT | O_WRONLY | O_TEXT);
#endif

    CAL_SetupMapFile ();
    CAL_SetupGrFile ();
    //CAL_SetupAudioFile ();

    mapon = -1;
}

//==========================================================================


/*
======================
=
= CA_Shutdown
=
= Closes all files
=
======================
*/

void CA_Shutdown (void)
{
#ifdef VBT	
    int i,start;

    if(maphandle != -1)
        close(maphandle);
    if(grhandle != -1)
        close(grhandle);
    if(audiohandle != -1)
        close(audiohandle);

    for(i=0; i<NUMCHUNKS; i++)
        UNCACHEGRCHUNK(i);
    free(pictable);

    switch(oldsoundmode)
    {
        case sdm_Off:
            return;
        case sdm_PC:
            start = STARTPCSOUNDS;
            break;
        case sdm_AdLib:
            start = STARTADLIBSOUNDS;
            break;
    }

    for(i=0; i<NUMSOUNDS; i++,start++)
        UNCACHEAUDIOCHUNK(start);
#endif
}

//===========================================================================

/*
======================
=
= CA_CacheAudioChunk
=
======================
*/

int32_t CA_CacheAudioChunk (int chunk)
{
#ifdef VBT
    int32_t pos = audiostarts[chunk];
    int32_t size = audiostarts[chunk+1]-pos;
	uint8_t *Chunks; 

//	slPrint("                                    ",slLocate(2,17));
//	slPrint("CA_CacheAudioChunk",slLocate(2,17));


    if (audiosegs[chunk])
        return size;                        // already in memory

    audiosegs[chunk]=(byte *) malloc(size);
    CHECKMALLOCRESULT(audiosegs[chunk]);
//hello
    //lseek(audiohandle,pos,SEEK_SET);
    //read(audiohandle,audiosegs[chunk],size);

	uint16_t delta;
	uint32_t delta2;

	delta = (uint16_t)(pos/2048);
	delta2 = (pos - delta*2048); 
	Chunks=(uint8_t*)malloc(0x4000);

	GFS_Load(audiohandle, delta, (void *)Chunks, size);

	Chunks=(uint8_t*)malloc(2048+size);
	//slPrint("                                    ",slLocate(2,17));
	//slPrint("CAL_GetGrChunkLength",slLocate(2,17));
	CHECKMALLOCRESULT(Chunks);
	GFS_Load(grhandle, delta, (void *)Chunks, 2048+size);

	memcpy(audiosegs[chunk],&Chunks[delta2],size);
	free(Chunks);
 
    return size;
#endif
	return 0;
}

void CA_CacheAdlibSoundChunk (int chunk)
{
	//c'est la fonction utilis�e
#ifdef VBT
	slPrint("                                         ",slLocate(2,17));
	slPrint("CA_CacheAdlibSoundChunk",slLocate(2,17));

    int32_t pos = audiostarts[chunk];
    int32_t size = audiostarts[chunk+1]-pos;
	uint8_t *Chunks; 

    if (audiosegs[chunk])
        return size;                        // already in memory

    audiosegs[chunk]=(byte *) malloc(size);
    CHECKMALLOCRESULT(audiosegs[chunk]);

	uint16_t delta;
	uint32_t delta2;

	delta = (uint16_t)(pos/2048);
	delta2 = (pos - delta*2048); 
	Chunks=(uint8_t*)malloc(2048+ORIG_ADLIBSOUND_SIZE - 1);

	GFS_Load(audiohandle, delta, (void *)Chunks, 2048+ORIG_ADLIBSOUND_SIZE - 1);

	Chunks=(uint8_t*)malloc(2048+size);
	//slPrint("                                    ",slLocate(2,17));
	//slPrint("CAL_GetGrChunkLength",slLocate(2,17));
	CHECKMALLOCRESULT(Chunks);
	GFS_Load(audiohandle, delta, (void *)Chunks, 2048+ORIG_ADLIBSOUND_SIZE - 1);
//    read(audiohandle, bufferseg, ORIG_ADLIBSOUND_SIZE - 1);   // without data[1]

	memcpy(audiosegs[chunk],&Chunks[delta2],size);
	free(Chunks);
#endif



#ifdef VBT
    int32_t pos = audiostarts[chunk];
    int32_t size = audiostarts[chunk+1]-pos;

    if (audiosegs[chunk])
        return;                        // already in memory
    lseek(audiohandle, pos, SEEK_SET);
    read(audiohandle, bufferseg, ORIG_ADLIBSOUND_SIZE - 1);   // without data[1]

    AdLibSound *sound = (AdLibSound *) malloc(size + sizeof(AdLibSound) - ORIG_ADLIBSOUND_SIZE);
    CHECKMALLOCRESULT(sound);

    byte *ptr = (byte *) bufferseg;
    sound->common.length = READLONGWORD(ptr);
    sound->common.priority = READWORD(ptr);
    sound->inst.mChar = *ptr++;
    sound->inst.cChar = *ptr++;
    sound->inst.mScale = *ptr++;
    sound->inst.cScale = *ptr++;
    sound->inst.mAttack = *ptr++;
    sound->inst.cAttack = *ptr++;
    sound->inst.mSus = *ptr++;
    sound->inst.cSus = *ptr++;
    sound->inst.mWave = *ptr++;
    sound->inst.cWave = *ptr++;
    sound->inst.nConn = *ptr++;
    sound->inst.voice = *ptr++;
    sound->inst.mode = *ptr++;
    sound->inst.unused[0] = *ptr++;
    sound->inst.unused[1] = *ptr++;
    sound->inst.unused[2] = *ptr++;
    sound->block = *ptr++;

    read(audiohandle, sound->data, size - ORIG_ADLIBSOUND_SIZE + 1);  // + 1 because of byte data[1]

    audiosegs[chunk]=(byte *) sound;
#endif
}

//===========================================================================

/*
======================
=
= CA_LoadAllSounds
=
= Purges all sounds, then loads all new ones (mode switch)
=
======================
*/

void CA_LoadAllSounds (void)
{
	//slPrint("                                         ",slLocate(2,17));
	//slPrint("CA_LoadAllSounds",slLocate(2,17));
#ifdef VBT
    unsigned start,i;

    switch (oldsoundmode)
    {
        case sdm_Off:
            goto cachein;
        case sdm_PC:
            start = STARTPCSOUNDS;
            break;
        case sdm_AdLib:
            start = STARTADLIBSOUNDS;
            break;
    }

    for (i=0;i<NUMSOUNDS;i++,start++)
        UNCACHEAUDIOCHUNK(start);

cachein:

    oldsoundmode = SoundMode;

    switch (SoundMode)
    {
        case sdm_Off:
			//slPrint("sdm_Off",slLocate(10,10));
            start = STARTADLIBSOUNDS;   // needed for priorities...
            break;
        case sdm_PC:
			//slPrint("sdm_PC",slLocate(10,10));
            start = STARTPCSOUNDS;
            break;
        case sdm_AdLib:
			//slPrint("sdm_AdLib",slLocate(10,10));
            start = STARTADLIBSOUNDS;
            break;
		default:
			//slPrint("nothing",slLocate(10,10));
			break;
    }

    if(start == STARTADLIBSOUNDS)
    {
        for (i=0;i<NUMSOUNDS;i++,start++)
            CA_CacheAdlibSoundChunk(start);
    }
    else
    {
        for (i=0;i<NUMSOUNDS;i++,start++)
            CA_CacheAudioChunk(start);
    }
#endif
}

//===========================================================================


/*
======================
=
= CAL_ExpandGrChunk
=
= Does whatever is needed with a pointer to a compressed chunk
=
======================
*/

void CAL_ExpandGrChunk (int chunk, int32_t *source)
{
	//slPrint("                                         ",slLocate(2,17));
	//slPrint("CAL_ExpandGrChunk",slLocate(2,17));

    int32_t    expanded;
    if (chunk >= STARTTILE8 && chunk < STARTEXTERNS)
    {
        //
        // expanded sizes of tile8/16/32 are implicit
        //

#define BLOCK           64
#define MASKBLOCK       128
        if (chunk<STARTTILE8M)          // tile 8s are all in one chunk!
            expanded = BLOCK*NUMTILE8;
        else if (chunk<STARTTILE16)
            expanded = MASKBLOCK*NUMTILE8M;
        else if (chunk<STARTTILE16M)    // all other tiles are one/chunk
            expanded = BLOCK*4;
        else if (chunk<STARTTILE32)
            expanded = MASKBLOCK*4;
        else if (chunk<STARTTILE32M)
            expanded = BLOCK*16;
        else
            expanded = MASKBLOCK*16;
    }
    else
    {
        //
        // everything else has an explicit size longword
        //
        expanded = *source++;
	expanded=SWAP_BYTES_32(expanded); 
    }
    //
    // allocate final space, decompress it, and free bigbuffer
    // Sprites need to have shifts made and various other junk
    //
//slPrint("           ",slLocate(2,21));	
//slPrintHex(expanded,slLocate(2,21));	
    grsegs[chunk]=(byte *) malloc(expanded);
    CHECKMALLOCRESULT(grsegs[chunk]);
	/*
	for(int i=0;i<30;i++)
		slPrintHex(source[i],slLocate(10,i+2));	 	*/
	 
    CAL_HuffExpand((byte *) source, grsegs[chunk], expanded, grhuffman);
 /*   fontstruct *font;
		for (int x=0; x<2;x++ )
	{
 		font = (fontstruct *) grsegs[STARTFONT+x];
		//font->height = SWAP_BYTES_16(font->height);

		for (int vbt=0; vbt<256;vbt++ )
		{
			font->location[vbt]=SWAP_BYTES_16(font->location[vbt]);
		}

	}
   */

//slPrint("CAL_ExpandGrChunk",slLocate(10,17));
}


/*
======================
=
= CA_CacheGrChunk
=
= Makes sure a given chunk is in memory, loadiing it if needed
=
======================
*/

void CA_CacheGrChunk (int chunk)
{
	//slPrint("                                         ",slLocate(2,17));
	//slPrint("CA_CacheGrChunk",slLocate(2,17));

    int32_t pos,compressed;
    int32_t *source;
    int  next;

    if (grsegs[chunk])
        return;                             // already in memory
//slPrint("CA_CacheGrChunk",slLocate(10,15));

//
// load the chunk into a buffer, either the miscbuffer if it fits, or allocate
// a larger buffer
//
    pos = GRFILEPOS(chunk);
//slPrintHex(pos,slLocate(5,6));
//	slPrint("          ",slLocate(2,17));
//	slPrintHex(pos,slLocate(2,17));

    if (pos<0)                              // $FFFFFFFF start is a sparse tile
        return;
    next = chunk +1;
    while (GRFILEPOS(next) == -1)           // skip past any sparse tiles
        next++;
    compressed = GRFILEPOS(next)-pos;
    //lseek(grhandle,pos,SEEK_SET);
		uint8_t *Chunks;
		uint16_t delta;
		uint32_t delta2;
		Uint16 i,j=0;
		delta = (uint16_t)(pos/2048);
		delta2 = (pos - delta*2048); 
	Chunks=(uint8_t*)malloc(0x4000);
	CHECKMALLOCRESULT(Chunks);
//	slPrint("g",slLocate(2,16));
//	slPrint("          ",slLocate(2,18));
//	slPrintHex(delta,slLocate(2,18));
//	slPrint("          ",slLocate(2,19));
//	slPrintHex(delta2,slLocate(2,19));
//	slPrint("          ",slLocate(2,20));
//	slPrintHex(pos,slLocate(2,20));
	GFS_Load(grhandle, delta, (void *)Chunks, 0x4000);
	///memset(Chunks,0,sizeof(Chunks));
    //slSynch();
    if (compressed<=BUFFERSIZE)
    {
//	slPrint("h",slLocate(2,16));
        //read(grhandle,bufferseg,compressed);
		//memcpy((byte*)bufferseg,&Chunks[delta2],compressed);
		for(i=0;i<compressed;i+=4)
		{
			//slPrintHex(j,slLocate(20,25));
			//bufferseg[j]=Chunks[i+3+delta2] | (Chunks[i+2+delta2]<<8)|(Chunks[i+1+delta2]<<16) | (Chunks[i+0+delta2]<<24);//SWAP_BYTES_16(Chunks[i+delta2+1]);
			//bufferseg[j]=Chunks[i+delta2] | (Chunks[i+1+delta2]<<8)|(Chunks[i+2+delta2]<<16) | (Chunks[i+3+delta2]<<24);//SWAP_BYTES_16(Chunks[i+delta2+1]);
			//bufferseg[j]=SWAP_BYTES_32(bufferseg[j]);
			bufferseg[j]=Chunks[i+3+delta2] | (Chunks[i+2+delta2]<<8)|(Chunks[i+1+delta2]<<16) | (Chunks[i+0+delta2]<<24);//SWAP_BYTES_16(Chunks[i+delta2+1]);
			j++;
		}
		//slPrintHex(bufferseg[1],slLocate(20,25));
        source = bufferseg;
/*
		for(i=0;i<5;i++)
		slPrint("           ",slLocate(24,i+3));		 

		for(i=0;i<5;i++)
		slPrintHex(source[i],slLocate(24,i+3));
*/		/*
		while(1);*/	
    }
    else
    {
//	slPrint("i",slLocate(2,16));
        source = (int32_t *) malloc(compressed);
        CHECKMALLOCRESULT(source);
        //read(grhandle,source,compressed);
		//memcpy((byte*)source,&Chunks[delta2],compressed);

	   
		for(i=0;i<compressed;i+=4)
		{
			//source[j]=Chunks[i+delta2] | (Chunks[i+1+delta2]<<8)|(Chunks[i+2+delta2]<<16) | (Chunks[i+3+delta2]<<24);//SWAP_BYTES_16(Chunks[i+delta2+1]);
			//source[j]=SWAP_BYTES_32(source[j]);
			source[j]=Chunks[i+3+delta2] | (Chunks[i+2+delta2]<<8)|(Chunks[i+1+delta2]<<16) | (Chunks[i+0+delta2]<<24);//SWAP_BYTES_16(Chunks[i+delta2+1]);
			j++;
		}	  
		  
    }
	free(Chunks);
	//slPrint("j",slLocate(2,16));
    CAL_ExpandGrChunk (chunk,source);

	//slPrint("k",slLocate(2,16));

    if (compressed>BUFFERSIZE)
        free(source);
	//slSynch();
}



//==========================================================================

/*
======================
=
= CA_CacheScreen
=
= Decompresses a chunk from disk straight onto the screen
=
======================
*/

void CA_CacheScreen (int chunk)
{
    int32_t    pos,compressed,expanded;
    memptr  bigbufferseg;
    int32_t    *source;
    int             next;

	//slPrint("                                         ",slLocate(2,17));
	//slPrint("CA_CacheScreen",slLocate(2,17));
//
// load the chunk into a buffer
//
    pos = GRFILEPOS(chunk);
    next = chunk +1;

    while (GRFILEPOS(next) == -1)           // skip past any sparse tiles
        next++;
    compressed = GRFILEPOS(next)-pos;

 //#ifdef VBT

    //lseek(grhandle,pos,SEEK_SET);

    bigbufferseg=malloc(compressed);
    CHECKMALLOCRESULT(bigbufferseg);
    //read(grhandle,bigbufferseg,compressed);

	uint8_t *Chunks;
	uint16_t delta;
	uint32_t delta2;
	Uint16 i,j=0;
	delta = (uint16_t)(pos/2048);
	delta2 = (pos - delta*2048); 
	Chunks=(uint8_t*)malloc(8192*2+compressed);
	//slPrint("f",slLocate(2,17));
	CHECKMALLOCRESULT(Chunks);
	GFS_Load(grhandle, delta, (void *)Chunks, 8192*2+compressed);

	memcpy(bigbufferseg,&Chunks[delta2],compressed);
	free(Chunks);

    source = (int32_t *) bigbufferseg;

    expanded = *source++;
	expanded = SWAP_BYTES_32(expanded);

//
// allocate final space, decompress it, and free bigbuffer
// Sprites need to have shifts made and various other junk
//
    byte *pic = (byte *) malloc(64000);
    CHECKMALLOCRESULT(pic);
    CAL_HuffExpand((byte *) source, pic, expanded, grhuffman);

    byte *vbuf = LOCK();
    for(int y = 0, scy = 0; y < 200; y++, scy += scaleFactor)
    {
        for(int x = 0, scx = 0; x < 320; x++, scx += scaleFactor)
        {
            byte col = pic[(y * 80 + (x >> 2)) + (x & 3) * 80 * 200];
            for(unsigned i = 0; i < scaleFactor; i++)
                for(unsigned j = 0; j < scaleFactor; j++)
                    vbuf[(scy + i) * curPitch + scx + j] = col;
        }
    }
    UNLOCK();
    free(pic);
    free(bigbufferseg);
//#endif
}

//==========================================================================

/*
======================
=
= CA_CacheMap
=
= WOLF: This is specialized for a 64*64 map size
=
======================
*/

void CA_CacheMap (int mapnum)
{
//	slPrint("                                         ",slLocate(2,17));
//	slPrint("CA_CacheMap",slLocate(2,17));
    int32_t   pos,compressed;
    int       plane;
    word     *dest;
    memptr    bigbufferseg;
    unsigned  size;
    byte     *source;
#ifdef CARMACIZED
    word     *buffer2seg;
    int32_t   expanded;
#endif

    mapon = mapnum;

//
// load the planes into the allready allocated buffers
//

	//slPrint("CA_CacheMap",slLocate(10,5));


    size = maparea*2;

    for (plane = 0; plane<MAPPLANES; plane++)
    {
        pos = mapheaderseg[mapnum]->planestart[plane];
        compressed = mapheaderseg[mapnum]->planelength[plane];
/*
slPrintHex(mapnum,slLocate(10,19));
slPrintHex(pos,slLocate(10,20));
slPrintHex(compressed,slLocate(10,21));
  */
        dest = mapsegs[plane];

        //lseek(maphandle,pos,SEEK_SET);
        if (compressed<=BUFFERSIZE)
		{
//slPrint("xxxx",slLocate(10,22));
            source = (byte *) bufferseg;
		}
        else
        {
//slPrint("yyyy",slLocate(10,22));
            bigbufferseg=malloc(compressed);
            CHECKMALLOCRESULT(bigbufferseg);
            source = (byte *) bigbufferseg;
        }


	Uint8 *vbt;
	int32_t i;
	long fileSize = GetFileSize(maphandle);
	vbt = (Uint8*)malloc(fileSize);
	CHECKMALLOCRESULT(vbt);
	GFS_Load(maphandle, 0, (void *)vbt, fileSize);

	memcpy(source,&vbt[pos],compressed);
	free(vbt);
        //read(maphandle,source,compressed);
#ifdef CARMACIZED


        //
        // unhuffman, then unRLEW
        // The huffman'd chunk has a two byte expanded length first
        // The resulting RLEW chunk also does, even though it's not really
        // needed
        //
        //expanded = *source;
		expanded = source[0] | (source[1] << 8);	
		//slPrintHex(expanded,slLocate(5,3));
        source++;
		source++;
        buffer2seg = (word *) malloc(expanded);
        CHECKMALLOCRESULT(buffer2seg);

        CAL_CarmackExpand((byte *) source, buffer2seg,expanded);
		// VBT valeur perdue ?????
	   RLEWtag=0xabcd;
        CA_RLEWexpand(buffer2seg+1,dest,size,RLEWtag);
        free(buffer2seg);

#else
        //
        // unRLEW, skipping expanded length
        //
        CA_RLEWexpand (source+1,dest,size,RLEWtag);
#endif

        if (compressed>BUFFERSIZE)
            free(bigbufferseg);
    }
}

//===========================================================================

void CA_CannotOpen(const char *string)
{
    char str[30];

    strcpy(str,"Can't open ");
    strcat(str,string);
    strcat(str,"!\n");
    Quit (str);
}
