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

// 圧縮データの情報構造体
struct LZSS_ENCODE_INFO
{
    int OriginalSize;  // 圧縮前のデータのサイズ(バイト数)
    int PressSize;     // 圧縮後のデータのサイズ(この構造体のサイズも含む)
    int EncodeCode;    // 圧縮情報の開始を示す数値
};


// 戻り値:圧縮後のサイズ  -1 はエラー  dest に NULL を入れると圧縮データ格納に必要なサイズが返る
unsigned int lzssEncode(const void *src, int srcSize, void *dest )
{
    int i ;
    unsigned char *SrcPoint, *DestPoint ;
    int PressSizeCounter, SrcSizeCounter ;
    LZSS_ENCODE_INFO EncodeInfo ;
    int EncodeCode ;

    // void 型のポインタではアドレスの操作が出来ないので unsigned char 型のポインタにする
    SrcPoint = ( unsigned char * )src ;
    DestPoint = ( unsigned char * )dest ;

    // 圧縮元データの中で一番出現頻度が低い数値を算出する
    {
        int NumCounter[256] ;  // １バイトで表現できる数値は０～２５５の２５６種類
        int MinNum ;

        // カウンターを初期化
        for( i = 0 ; i < 256 ; i ++ )
        {
            NumCounter[i] = 0 ;
        }

        // 出現回数をカウント
        for( i = 0 ; i < srcSize ; i ++ )
        {
            NumCounter[ SrcPoint[i] ] ++ ;
        }

        // 一番カウンターの値が少ない数値を圧縮情報の開始を示す数値に任命
        {
            // 最初は仮に０番を開始を示す数値としておく
            EncodeCode = 0 ;
            MinNum = NumCounter[0] ;
            for( i = 1 ; i < 256 ; i ++ )
            {
                // より出現数が低い数値が見つかったら更新
                if( MinNum > NumCounter[i] )
                {
                    MinNum = NumCounter[i] ;
                    EncodeCode = i ;
                }
            }
        }
    }

    // 圧縮処理
    {
        int Index, EqualNum, MaxEqualNum, MaxIndex ;
        unsigned char *PressData ;
    
        // 圧縮データを格納するアドレスをセット
        // (圧縮データ本体は元のサイズ、圧縮後のサイズ、圧縮情報の開始を示す数値を
        // 格納するデータ領域の後になる)
        PressData = DestPoint + sizeof( LZSS_ENCODE_INFO ) ;
        
        // 圧縮するデータの参照アドレスを初期化
        SrcSizeCounter = 0 ;
        
        // 圧縮したデータの格納アドレスを初期化
        PressSizeCounter = 0 ;
        
        // 全てのデータを圧縮するまで繰り返す
        while( SrcSizeCounter < srcSize )
        {
            // 今までに同じ数値の羅列がなかったか調べる
            {
                MaxEqualNum = -1 ;
                MaxIndex = -1 ;
                
                // 最高で２５４バイト前まで調べる
                for( Index = 1 ; Index <= 254 ; Index ++ )
                {
                    // データの先頭より前を調べようとしていたら抜ける
                    if( SrcSizeCounter - Index < 0 ) break ;
                
                    // 同じ数値が何回続いているか調べる
                    for( EqualNum = 0 ; EqualNum < Index ; EqualNum ++ )
                    {
                        // 圧縮元データの最後に到達したらループを抜ける
                        if( EqualNum + SrcSizeCounter >= srcSize ) break ;
                        
                        // 数値が違ったらループを抜ける
                        if( SrcPoint[SrcSizeCounter + EqualNum] != SrcPoint[SrcSizeCounter - Index + EqualNum] ) break ;
                    }
                    
                    // 同じだった回数が４以上(４未満だと逆にサイズが増える)で、且
                    // 今まで見つけた回数よりも多い場合に参照アドレスを更新する
                    if( EqualNum >= 4 && MaxEqualNum < EqualNum )
                    {
                        MaxEqualNum = EqualNum ;
                        MaxIndex = Index ;
                    }
                }
            }

            // 同じ数値の羅列が見つからなかったら普通に出力
            if( MaxIndex == -1 )
            {
                if( dest != NULL ) PressData[PressSizeCounter] = SrcPoint[SrcSizeCounter] ;
                PressSizeCounter ++ ;

                // 圧縮情報の開始を示す数値と同じだった場合は２回連続で出力する
                if( SrcPoint[SrcSizeCounter] == EncodeCode )
                {
                    if( dest != NULL ) PressData[PressSizeCounter] = SrcPoint[SrcSizeCounter] ;
                    PressSizeCounter ++ ;
                }
                
                // 圧縮元データの参照アドレスを一つ進める                
                SrcSizeCounter ++ ;
            }
            else
            {
                // 見つかった場合は見つけた位置と長さを出力する

                // 最初に圧縮情報の開始を示す数値を出力する
                if( dest != NULL ) PressData[PressSizeCounter] = ( unsigned char )EncodeCode ;
                PressSizeCounter ++ ;
                
                // 次に『何バイト前からが同じか？』の数値を出力する
                {
                    if( dest != NULL ) PressData[PressSizeCounter] = ( unsigned char )MaxIndex ;
                    
                    // もし圧縮情報の開始を示す数値と同じ場合、展開時に
                    // 『圧縮情報の開始を示す数値そのもの』と判断されて
                    // しまうため、圧縮情報の開始を示す数値と同じかそれ以上の
                    // 場合は数値を +1 するというルールを使う。(展開時は逆に -1 にする)
                    if( dest != NULL && PressData[PressSizeCounter] >= EncodeCode )
                        PressData[PressSizeCounter] ++ ;
                    
                    PressSizeCounter ++ ;
                }
                
                // 次に『何バイト同じか？』の数値を出力する
                if( dest != NULL ) PressData[PressSizeCounter] = ( unsigned char )MaxEqualNum ;
                PressSizeCounter ++ ;
                
                // 同じだったバイト数分だけ圧縮元データの参照アドレスを進める
                SrcSizeCounter += MaxEqualNum ;
            }
        }
    }
    
    // 圧縮データの情報をセットする
    {
        // 圧縮前のデータのサイズをセット
        EncodeInfo.OriginalSize  = srcSize ;
        
        // 圧縮後のデータのサイズをセット
        EncodeInfo.PressSize     = PressSizeCounter + sizeof( LZSS_ENCODE_INFO ) ;
        
        // 圧縮情報の開始を知らせる数値をセット
        EncodeInfo.EncodeCode    = EncodeCode ;
    }
    
    // 圧縮データの情報を圧縮データにコピーする
    if( dest != NULL ) memcpy( DestPoint, &EncodeInfo, sizeof( LZSS_ENCODE_INFO ) ) ;

    // 圧縮後のデータのサイズを返す
    return EncodeInfo.PressSize ;
}

// 圧縮データを解凍する
//
// 戻り値:解凍後のサイズ  -1 はエラー  Dest に NULL を入れると解凍データ格納に必要なサイズが返る
unsigned int lzssDecode(const void *Press, void *Dest)
{
    int PressSize, PressSizeCounter, DestSizeCounter ;
    unsigned char *PressPoint, *DestPoint ;
    LZSS_ENCODE_INFO EncodeInfo ;

    // void 型のポインタではアドレスの操作が出来ないので unsigned char 型のポインタにする
    PressPoint = ( unsigned char * )Press ;
    DestPoint = ( unsigned char * )Dest ;
    
    // 圧縮データの情報を取得する
    memcpy( &EncodeInfo, Press, sizeof( LZSS_ENCODE_INFO ) ) ;
    
    // Dest が NULL の場合は 解凍後のデータのサイズを返す
    if( Dest == NULL )
        return EncodeInfo.OriginalSize ;

    // 圧縮データ本体のサイズを取得する
    PressSize = EncodeInfo.PressSize - sizeof( LZSS_ENCODE_INFO ) ;
    
    // 解凍処理
    {
        int Index, EqualNum ;
        unsigned char *PressData ;
        
        // 圧縮データ本体の先頭アドレスをセット
        // (圧縮データ本体は元のサイズ、圧縮後のサイズ、圧縮情報の開始を示す数値を
        // 格納するデータ領域の後にある)
        PressData = PressPoint + sizeof( LZSS_ENCODE_INFO ) ;
        
        // 解凍したデータを格納するアドレスの初期化
        DestSizeCounter = 0 ;
        
        // 解凍する圧縮データの参照アドレスの初期化
        PressSizeCounter = 0 ;
        
        // 全ての圧縮データを解凍するまでループ
        while( PressSizeCounter < PressSize )
        {
            // 圧縮情報の開始を示す数値かどうかで処理を分岐
            if( PressData[PressSizeCounter] == EncodeInfo.EncodeCode )
            {
                PressSizeCounter ++ ;
            
                // ２バイト連続で圧縮情報の開始を示す数値だった場合、開始を示す
                // 数値そのものを示しているのでそのまま出力する
                if( PressData[PressSizeCounter] == EncodeInfo.EncodeCode )
                {
                    DestPoint[DestSizeCounter] = ( unsigned char )EncodeInfo.EncodeCode ;
                    DestSizeCounter ++ ;

                    PressSizeCounter ++ ;
                }
                else
                {
                    // 普通に圧縮情報を示す数値だった場合
                    
                    // 『何バイト前から？』の数値を得る
                    {
                        Index = PressData[PressSizeCounter] ;
                        PressSizeCounter ++ ;
                        
                        // 『何バイト前から？』の数値が圧縮情報を示す数値より
                        // 大きい値の場合は－１する(詳しくはエンコードプログラムを参照してください)
                        if( Index > EncodeInfo.EncodeCode ) Index -- ;
                    }
                    
                    // 『何バイト同じか？』の数値を得る
                    EqualNum = PressData[PressSizeCounter] ;
                    PressSizeCounter ++ ;
                    
                    // 指定のバイト数分だけ前のアドレスから、指定の
                    // バイト数分だけコピー
                    memcpy( &DestPoint[DestSizeCounter], &DestPoint[DestSizeCounter - Index], EqualNum ) ;
                    DestSizeCounter += EqualNum ;
                }
            }
            else
            {
                // 普通に数値を出力
                DestPoint[DestSizeCounter] = PressData[PressSizeCounter] ;

                DestSizeCounter ++ ;
                PressSizeCounter ++ ;
            }
        }
    }
    
    // 解凍後のサイズを返す
    return EncodeInfo.OriginalSize ;
}


}//namespace rtl
}//namespace bzs

