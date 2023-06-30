#if !defined(LINUX)

#include "XtfImage.h"

CImageXtfFile::CImageXtfFile(byte* ptr, long filesize)
{
/*	SXtfHeader *pMyTexture = (SXtfHeader *)ptr;
	u_char *pRawData = ptr+sizeof(SXtfHeader);


	//Check type
	if(pMyTexture->Type!=XTF_FILE_TYPE)
	{
		mfSet_error(eIFE_BadFormat,"File Type Error");
	}

	//Check version
	if(pMyTexture->Version!=XTF_FILE_VERSION)
	{
		mfSet_error(eIFE_BadFormat,"File Version Error");
	}

	mfSet_dimensions(pMyTexture->Info.m_Width,pMyTexture->Info.m_Height);

	//Set the image raw data
	SRGBPixel *CurImage = mfGet_image();

	memcpy(CurImage,MyTexture->Info.m_pData32,MyTexture->Info.m_Width*MyTexture->Info.m_Height*2);

	mfSet_Flags(MyTexture->Info.Flags);
*/
	OutputDebugString("Constructor non supported");
	mfSet_error(eIFE_BadFormat,"");
}

CImageXtfFile::~CImageXtfFile ()
{
}

bool CImageXtfFile::CImageXtfFileSave(const char *name, STexPic *pTexture)
{
#ifdef DEBUG
	SXtfHeader	MyTexture;
	FILE *MyFile;
	char NewName[255];

	if(name)
	{
		strcpy(NewName,name);
		if(NewName[strlen(NewName)-4]=='.')
			NewName[strlen(NewName)-4]='\0';
		else
		if(NewName[strlen(NewName)-3]=='.')
			NewName[strlen(NewName)-3]='\0';
	}
	else
	{
		memset(NewName,0,255);
		int PathLength=strlen(pTexture->m_Pak);
		int i=0;
		while((i<PathLength) && pTexture->m_Pak[i]!='.' )
		{
			NewName[i]=pTexture->m_Pak[i];
			i++;
		}
		if (pTexture->m_Pak[i]=='.')
		{
			while((i>=0) && pTexture->m_Pak[i]!='\\' )
			{
				i--;
			}
			NewName[i]=0;
		}
		strcat(NewName,"\\");
		strcat(NewName,pTexture->m_Name);
	}


	strcat(NewName,".xtf");
	MyFile=fopen(NewName,"w");
	if(!MyFile)
		return false;

	MyTexture.Type = XTF_FILE_TYPE;
	MyTexture.Version = XTF_FILE_VERSION;
	MyTexture.Info = *pTexture;

	fwrite(&MyTexture,sizeof(SXtfHeader),1,MyFile);
	fwrite((const void *)pTexture->m_pData32,pTexture->m_Width*pTexture->m_Height*2,1,MyFile);

	fclose(MyFile);
	return true;
#endif
	return false;
}

bool CImageXtfFile::CImageXtfFileSave(const char *name, u_char *pTexture, u_int dimx, u_int dimy)
{
#ifdef DEBUG
	SXtfHeader	MyTexture;
	FILE *MyFile;
	char NewName[255];

	strcpy(NewName,name);
	NewName[strlen(NewName)-4]='\0';
	strcat(NewName,".xtf");
	MyFile=fopen(NewName,"w");
	if(!MyFile)
		return false;

	MyTexture.Type = XTF_FILE_TYPE;
	MyTexture.Version = XTF_FILE_VERSION;
	MyTexture.Info.m_Width=dimx;
	MyTexture.Info.m_Height=dimy;
	MyTexture.Info.m_pData32=pTexture;

	fwrite(&MyTexture,sizeof(SXtfHeader),1,MyFile);
	fwrite((const void *)MyTexture.Info.m_pData32,MyTexture.Info.m_Width*MyTexture.Info.m_Height*2,1,MyFile);

	fclose(MyFile);
	return true;
#endif
	return false;
}


///Load the texture in .xtf file format
STexPic* CImageXtfFile::CImageXtfFileLoad (const char *FileName)
{
	FILE 		*TextureFile;
	SXtfHeader	*MyTexture;
	STexPic		*pTexturePic;
	u_char		*pRawData;

	TextureFile=fopen(FileName,"r");
	if(!TextureFile)
	{
		mfSet_error(eIFE_BadFormat,"File Not Found");
		return NULL;
	}
	u_int FileSize=CXFile::GetLength(FileName);


	MyTexture = new SXtfHeader;
	pRawData = new u_char[FileSize-sizeof(SXtfHeader)];
	fread(MyTexture,sizeof(SXtfHeader),1,TextureFile);
	fread(pRawData,FileSize-sizeof(SXtfHeader),1,TextureFile);


	//Check type
	if(MyTexture->Type!=XTF_FILE_TYPE)
	{
		delete MyTexture;
		fclose(TextureFile);
		mfSet_error(eIFE_BadFormat,"File Type Error");
		return NULL;
	}

	//Check version
	if(MyTexture->Version!=XTF_FILE_VERSION)
	{
		delete MyTexture;
		fclose(TextureFile);
		mfSet_error(eIFE_BadFormat,"File Version Error");
		return NULL;
	}


	MyTexture->Info.m_pData32=pRawData;

	fclose(TextureFile);
	mfSet_error(eIFE_OK,"");

	pTexturePic = new STexPic;
	*pTexturePic = MyTexture->Info;

	pTexturePic->m_WidthReal=MyTexture->Info.m_Width;
	pTexturePic->m_HeightReal=MyTexture->Info.m_Height;

	delete MyTexture;

	return pTexturePic;
}

#endif