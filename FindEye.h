#include "cv.h"
#include "highgui.h"
#include <stdio.h>
#include <ctype.h>

static class FindEye
{
public:
	static IplImage *image;
	IplImage *grey;
	IplImage *prev_grey;
	IplImage *pyramid;
	IplImage *prev_pyramid;
	IplImage *swap_temp;
	int win_size ;
	const int MAX_COUNT;
	CvPoint2D32f* points[2], *swap_points;
	char* status ;
	int count ;
	int flags;
	static int add_remove_pt ;
	static CvPoint pt;
	double outs[2][2];

	FindEye():MAX_COUNT(2)
	{
		win_size = 10 ;
		count = 0;
		 flags = 0;
		  add_remove_pt = 0;
	}
	
	~FindEye()
	{

	}
static void on_mouse( int event, int x, int y, int flags, void* param )
{
    if( !image )
        return;

    if( image->origin )
        y = image->height - y;

    if( event == CV_EVENT_LBUTTONDOWN )
    {
        pt = cvPoint(x,y);
        add_remove_pt = 1;
    }
}
	void Run()
	{
		CvCapture* capture = 0;
		CoInitialize(NULL);
		capture = cvCaptureFromCAM(0);
		 if( !capture )
    {
        fprintf(stderr,"Could not initialize capturing...\n");
        return ;
    }
		 cvSetCaptureProperty(capture,CV_CAP_PROP_FRAME_WIDTH,1024);
		 cvSetCaptureProperty(capture,CV_CAP_PROP_FRAME_HEIGHT,768);
		 cvNamedWindow( "FindEye", CV_WINDOW_AUTOSIZE );
	cvSetMouseCallback( "FindEye", on_mouse, 0 );

    for(;;)
    {
        IplImage* frame = 0;
        int i, k, c;

        frame = cvQueryFrame( capture );
		
        if( !frame )
            break;

        if( !image )
        {
            /* allocate all the buffers */
            image = cvCreateImage( cvGetSize(frame), 8, 3 );
            image->origin = frame->origin;
            grey = cvCreateImage( cvGetSize(frame), 8, 1 );
            prev_grey = cvCreateImage( cvGetSize(frame), 8, 1 );
            pyramid = cvCreateImage( cvGetSize(frame), 8, 1 );
            prev_pyramid = cvCreateImage( cvGetSize(frame), 8, 1 );
            points[0] = (CvPoint2D32f*)cvAlloc(MAX_COUNT*sizeof(points[0][0]));
            points[1] = (CvPoint2D32f*)cvAlloc(MAX_COUNT*sizeof(points[0][0]));
            status = (char*)cvAlloc(MAX_COUNT);
            flags = 0;
        }

        cvCopy( frame, image, 0 );
        cvCvtColor( image, grey, CV_BGR2GRAY );

 if( count > 0 )
        {
            cvCalcOpticalFlowPyrLK( prev_grey, grey, prev_pyramid, pyramid,
                points[0], points[1], count, cvSize(win_size,win_size), 3, status, 0,
                cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03), flags );
            flags |= CV_LKFLOW_PYR_A_READY;
			//printf("\n");
            for( i = k = 0; i < count; i++ )
            {
                if( add_remove_pt )
                {
                    double dx = pt.x - points[1][i].x;
                    double dy = pt.y - points[1][i].y;

                    if( dx*dx + dy*dy <= 25 )
                    {
                        add_remove_pt = 0;
                        continue;
                    }
                }

                if( !status[i] )
                    continue;

                points[1][k++] = points[1][i];
                cvCircle( image, cvPointFrom32f(points[1][i]), 3, CV_RGB(255,0,0), -1, 8,0);
				/*printf ("%d %d  ",
					cvPointFrom32f(points[1][i]).x,
	   cvPointFrom32f( points[1][i]).y);*/
				outs[i][0]=(double)cvPointFrom32f( points[1][i]).x/(double)image->width;
				outs[i][1]=(double)cvPointFrom32f( points[1][i]).y/(double)image->height;
				//printf("%i\n",outs);
            }
            count = k;
        }

        if( add_remove_pt && count < MAX_COUNT )
        {
            points[1][count++] = cvPointTo32f(pt);
            cvFindCornerSubPix( grey, points[1] + count - 1, 1,
                cvSize(win_size,win_size), cvSize(-1,-1),
                cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03));
            add_remove_pt = 0;
        }

        CV_SWAP( prev_grey, grey, swap_temp );
        CV_SWAP( prev_pyramid, pyramid, swap_temp );
        CV_SWAP( points[0], points[1], swap_points );
   
        cvShowImage( "FindEye", image );

        c = cvWaitKey(10);
        if( (char)c == 27 )
            break;
        switch( (char) c )
        {
        case 'c':
            count = 0;
            break;
        }
    }

    cvReleaseCapture( &capture );
    cvDestroyWindow("FindEye");

	}
	

};
 IplImage *FindEye::image=0;
 int FindEye::add_remove_pt=0; 
 CvPoint FindEye::pt;