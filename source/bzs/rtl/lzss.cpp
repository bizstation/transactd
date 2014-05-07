/*=================================================================
   Copyright (C) 2014 BizStation Corp All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software 
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
   02111-1307, USA.
=================================================================*/
#include<bzs/env/tstring.h>
#pragma hdrstop

#include "lzss.h"

#pragma package(smart_init)

namespace bzs
{
namespace rtl
{

// ���k�f�[�^�̏��\����
struct LZSS_ENCODE_INFO
{
    int OriginalSize;  // ���k�O�̃f�[�^�̃T�C�Y(�o�C�g��)
    int PressSize;     // ���k��̃f�[�^�̃T�C�Y(���̍\���̂̃T�C�Y���܂�)
    int EncodeCode;    // ���k���̊J�n���������l
};


// �߂�l:���k��̃T�C�Y  -1 �̓G���[  dest �� NULL ������ƈ��k�f�[�^�i�[�ɕK�v�ȃT�C�Y���Ԃ�
unsigned int lzssEncode(const void *src, int srcSize, void *dest )
{
    int i ;
    unsigned char *SrcPoint, *DestPoint ;
    int PressSizeCounter, SrcSizeCounter ;
    LZSS_ENCODE_INFO EncodeInfo ;
    int EncodeCode ;

    // void �^�̃|�C���^�ł̓A�h���X�̑��삪�o���Ȃ��̂� unsigned char �^�̃|�C���^�ɂ���
    SrcPoint = ( unsigned char * )src ;
    DestPoint = ( unsigned char * )dest ;

    // ���k���f�[�^�̒��ň�ԏo���p�x���Ⴂ���l���Z�o����
    {
        int NumCounter[256] ;  // �P�o�C�g�ŕ\���ł��鐔�l�͂O�`�Q�T�T�̂Q�T�U���
        int MinNum ;

        // �J�E���^�[��������
        for( i = 0 ; i < 256 ; i ++ )
        {
            NumCounter[i] = 0 ;
        }

        // �o���񐔂��J�E���g
        for( i = 0 ; i < srcSize ; i ++ )
        {
            NumCounter[ SrcPoint[i] ] ++ ;
        }

        // ��ԃJ�E���^�[�̒l�����Ȃ����l�����k���̊J�n���������l�ɔC��
        {
            // �ŏ��͉��ɂO�Ԃ��J�n���������l�Ƃ��Ă���
            EncodeCode = 0 ;
            MinNum = NumCounter[0] ;
            for( i = 1 ; i < 256 ; i ++ )
            {
                // ���o�������Ⴂ���l������������X�V
                if( MinNum > NumCounter[i] )
                {
                    MinNum = NumCounter[i] ;
                    EncodeCode = i ;
                }
            }
        }
    }

    // ���k����
    {
        int Index, EqualNum, MaxEqualNum, MaxIndex ;
        unsigned char *PressData ;
    
        // ���k�f�[�^���i�[����A�h���X���Z�b�g
        // (���k�f�[�^�{�̂͌��̃T�C�Y�A���k��̃T�C�Y�A���k���̊J�n���������l��
        // �i�[����f�[�^�̈�̌�ɂȂ�)
        PressData = DestPoint + sizeof( LZSS_ENCODE_INFO ) ;
        
        // ���k����f�[�^�̎Q�ƃA�h���X��������
        SrcSizeCounter = 0 ;
        
        // ���k�����f�[�^�̊i�[�A�h���X��������
        PressSizeCounter = 0 ;
        
        // �S�Ẵf�[�^�����k����܂ŌJ��Ԃ�
        while( SrcSizeCounter < srcSize )
        {
            // ���܂łɓ������l�̗��񂪂Ȃ����������ׂ�
            {
                MaxEqualNum = -1 ;
                MaxIndex = -1 ;
                
                // �ō��łQ�T�S�o�C�g�O�܂Œ��ׂ�
                for( Index = 1 ; Index <= 254 ; Index ++ )
                {
                    // �f�[�^�̐擪���O�𒲂ׂ悤�Ƃ��Ă����甲����
                    if( SrcSizeCounter - Index < 0 ) break ;
                
                    // �������l�����񑱂��Ă��邩���ׂ�
                    for( EqualNum = 0 ; EqualNum < Index ; EqualNum ++ )
                    {
                        // ���k���f�[�^�̍Ō�ɓ��B�����烋�[�v�𔲂���
                        if( EqualNum + SrcSizeCounter >= srcSize ) break ;
                        
                        // ���l��������烋�[�v�𔲂���
                        if( SrcPoint[SrcSizeCounter + EqualNum] != SrcPoint[SrcSizeCounter - Index + EqualNum] ) break ;
                    }
                    
                    // �����������񐔂��S�ȏ�(�S�������Ƌt�ɃT�C�Y��������)�ŁA��
                    // ���܂Ō������񐔂��������ꍇ�ɎQ�ƃA�h���X���X�V����
                    if( EqualNum >= 4 && MaxEqualNum < EqualNum )
                    {
                        MaxEqualNum = EqualNum ;
                        MaxIndex = Index ;
                    }
                }
            }

            // �������l�̗��񂪌�����Ȃ������畁�ʂɏo��
            if( MaxIndex == -1 )
            {
                if( dest != NULL ) PressData[PressSizeCounter] = SrcPoint[SrcSizeCounter] ;
                PressSizeCounter ++ ;

                // ���k���̊J�n���������l�Ɠ����������ꍇ�͂Q��A���ŏo�͂���
                if( SrcPoint[SrcSizeCounter] == EncodeCode )
                {
                    if( dest != NULL ) PressData[PressSizeCounter] = SrcPoint[SrcSizeCounter] ;
                    PressSizeCounter ++ ;
                }
                
                // ���k���f�[�^�̎Q�ƃA�h���X����i�߂�                
                SrcSizeCounter ++ ;
            }
            else
            {
                // ���������ꍇ�͌������ʒu�ƒ������o�͂���

                // �ŏ��Ɉ��k���̊J�n���������l���o�͂���
                if( dest != NULL ) PressData[PressSizeCounter] = ( unsigned char )EncodeCode ;
                PressSizeCounter ++ ;
                
                // ���Ɂw���o�C�g�O���炪�������H�x�̐��l���o�͂���
                {
                    if( dest != NULL ) PressData[PressSizeCounter] = ( unsigned char )MaxIndex ;
                    
                    // �������k���̊J�n���������l�Ɠ����ꍇ�A�W�J����
                    // �w���k���̊J�n���������l���̂��́x�Ɣ��f�����
                    // ���܂����߁A���k���̊J�n���������l�Ɠ���������ȏ��
                    // �ꍇ�͐��l�� +1 ����Ƃ������[�����g���B(�W�J���͋t�� -1 �ɂ���)
                    if( dest != NULL && PressData[PressSizeCounter] >= EncodeCode )
                        PressData[PressSizeCounter] ++ ;
                    
                    PressSizeCounter ++ ;
                }
                
                // ���Ɂw���o�C�g�������H�x�̐��l���o�͂���
                if( dest != NULL ) PressData[PressSizeCounter] = ( unsigned char )MaxEqualNum ;
                PressSizeCounter ++ ;
                
                // �����������o�C�g�����������k���f�[�^�̎Q�ƃA�h���X��i�߂�
                SrcSizeCounter += MaxEqualNum ;
            }
        }
    }
    
    // ���k�f�[�^�̏����Z�b�g����
    {
        // ���k�O�̃f�[�^�̃T�C�Y���Z�b�g
        EncodeInfo.OriginalSize  = srcSize ;
        
        // ���k��̃f�[�^�̃T�C�Y���Z�b�g
        EncodeInfo.PressSize     = PressSizeCounter + sizeof( LZSS_ENCODE_INFO ) ;
        
        // ���k���̊J�n��m�点�鐔�l���Z�b�g
        EncodeInfo.EncodeCode    = EncodeCode ;
    }
    
    // ���k�f�[�^�̏������k�f�[�^�ɃR�s�[����
    if( dest != NULL ) memcpy( DestPoint, &EncodeInfo, sizeof( LZSS_ENCODE_INFO ) ) ;

    // ���k��̃f�[�^�̃T�C�Y��Ԃ�
    return EncodeInfo.PressSize ;
}

// ���k�f�[�^���𓀂���
//
// �߂�l:�𓀌�̃T�C�Y  -1 �̓G���[  Dest �� NULL ������Ɖ𓀃f�[�^�i�[�ɕK�v�ȃT�C�Y���Ԃ�
unsigned int lzssDecode(const void *Press, void *Dest)
{
    int PressSize, PressSizeCounter, DestSizeCounter ;
    unsigned char *PressPoint, *DestPoint ;
    LZSS_ENCODE_INFO EncodeInfo ;

    // void �^�̃|�C���^�ł̓A�h���X�̑��삪�o���Ȃ��̂� unsigned char �^�̃|�C���^�ɂ���
    PressPoint = ( unsigned char * )Press ;
    DestPoint = ( unsigned char * )Dest ;
    
    // ���k�f�[�^�̏����擾����
    memcpy( &EncodeInfo, Press, sizeof( LZSS_ENCODE_INFO ) ) ;
    
    // Dest �� NULL �̏ꍇ�� �𓀌�̃f�[�^�̃T�C�Y��Ԃ�
    if( Dest == NULL )
        return EncodeInfo.OriginalSize ;

    // ���k�f�[�^�{�̂̃T�C�Y���擾����
    PressSize = EncodeInfo.PressSize - sizeof( LZSS_ENCODE_INFO ) ;
    
    // �𓀏���
    {
        int Index, EqualNum ;
        unsigned char *PressData ;
        
        // ���k�f�[�^�{�̂̐擪�A�h���X���Z�b�g
        // (���k�f�[�^�{�̂͌��̃T�C�Y�A���k��̃T�C�Y�A���k���̊J�n���������l��
        // �i�[����f�[�^�̈�̌�ɂ���)
        PressData = PressPoint + sizeof( LZSS_ENCODE_INFO ) ;
        
        // �𓀂����f�[�^���i�[����A�h���X�̏�����
        DestSizeCounter = 0 ;
        
        // �𓀂��鈳�k�f�[�^�̎Q�ƃA�h���X�̏�����
        PressSizeCounter = 0 ;
        
        // �S�Ă̈��k�f�[�^���𓀂���܂Ń��[�v
        while( PressSizeCounter < PressSize )
        {
            // ���k���̊J�n���������l���ǂ����ŏ����𕪊�
            if( PressData[PressSizeCounter] == EncodeInfo.EncodeCode )
            {
                PressSizeCounter ++ ;
            
                // �Q�o�C�g�A���ň��k���̊J�n���������l�������ꍇ�A�J�n������
                // ���l���̂��̂������Ă���̂ł��̂܂܏o�͂���
                if( PressData[PressSizeCounter] == EncodeInfo.EncodeCode )
                {
                    DestPoint[DestSizeCounter] = ( unsigned char )EncodeInfo.EncodeCode ;
                    DestSizeCounter ++ ;

                    PressSizeCounter ++ ;
                }
                else
                {
                    // ���ʂɈ��k�����������l�������ꍇ
                    
                    // �w���o�C�g�O����H�x�̐��l�𓾂�
                    {
                        Index = PressData[PressSizeCounter] ;
                        PressSizeCounter ++ ;
                        
                        // �w���o�C�g�O����H�x�̐��l�����k�����������l���
                        // �傫���l�̏ꍇ�́|�P����(�ڂ����̓G���R�[�h�v���O�������Q�Ƃ��Ă�������)
                        if( Index > EncodeInfo.EncodeCode ) Index -- ;
                    }
                    
                    // �w���o�C�g�������H�x�̐��l�𓾂�
                    EqualNum = PressData[PressSizeCounter] ;
                    PressSizeCounter ++ ;
                    
                    // �w��̃o�C�g���������O�̃A�h���X����A�w���
                    // �o�C�g���������R�s�[
                    memcpy( &DestPoint[DestSizeCounter], &DestPoint[DestSizeCounter - Index], EqualNum ) ;
                    DestSizeCounter += EqualNum ;
                }
            }
            else
            {
                // ���ʂɐ��l���o��
                DestPoint[DestSizeCounter] = PressData[PressSizeCounter] ;

                DestSizeCounter ++ ;
                PressSizeCounter ++ ;
            }
        }
    }
    
    // �𓀌�̃T�C�Y��Ԃ�
    return EncodeInfo.OriginalSize ;
}


}//namespace rtl
}//namespace bzs

