/*
* 
* Copyright (c) 2014, Wieden+Kennedy
* Stephen Schieberl
* All rights reserved.
* 
* Redistribution and use in source and binary forms, with or 
* without modification, are permitted provided that the following 
* conditions are met:
* 
* Redistributions of source code must retain the above copyright 
* notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright 
* notice, this list of conditions and the following disclaimer in 
* the documentation and/or other materials provided with the 
* distribution.
* 
* Neither the name of the Ban the Rewind nor the names of its 
* contributors may be used to endorse or promote products 
* derived from this software without specific prior written 
* permission.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
* COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* 
*/

#include "cinder/app/AppBasic.h"
#include "cinder/gl/Texture.h"
#include "cinder/params/Params.h"

#include "Kinect2.h"

class FaceApp : public ci::app::AppBasic 
{
public:
	void							draw();
	void							prepareSettings( ci::app::AppBasic::Settings* settings );
	void							setup();
	void							update();
private:
	std::vector<Kinect2::Body>		mBodies;
	ci::Channel8u					mChannelBodyIndex;
	ci::Channel16u					mChannelDepth;
	Kinect2::DeviceRef				mDevice;

	float							mFrameRate;
	bool							mFullScreen;
	ci::params::InterfaceGlRef		mParams;
};

using namespace ci;
using namespace ci::app;
using namespace std;

void FaceApp::draw()
{
	gl::setViewport( getWindowBounds() );
	gl::clear();
	gl::setMatricesWindow( getWindowSize() );

	if ( mChannelDepth ) {
		gl::enable( GL_TEXTURE_2D );
		gl::TextureRef tex = gl::Texture::create( Kinect2::channel16To8( mChannelDepth ) );
		gl::draw( tex, tex->getBounds(), Rectf( getWindowBounds() ) );
	}
	
	if ( mChannelBodyIndex ) {
		gl::enable( GL_TEXTURE_2D );
		gl::color( ColorAf( Colorf::white(), 0.15f ) );
		gl::TextureRef tex = gl::Texture::create( Kinect2::colorizeBodyIndex( mChannelBodyIndex ) );
		gl::draw( tex, tex->getBounds(), Rectf( getWindowBounds() ) );

		gl::color( ColorAf::white() );
		gl::pushMatrices();
		gl::scale( Vec2f( getWindowSize() ) / Vec2f( mChannelBodyIndex.getSize() ) );
		gl::disable( GL_TEXTURE_2D );
		for ( const Kinect2::Body& body : mBodies ) {
			if ( body.isTracked() ) {
				const Kinect2::Body::Face2d& face = body.getFace2d();
				if ( face.isTracked() ) {
					gl::drawStrokedRect( face.getBoundsInfrared() );
				}
			}
		}
		gl::popMatrices();
	}

	mParams->draw();
}

void FaceApp::prepareSettings( Settings* settings )
{
	settings->prepareWindow( Window::Format().size( 800, 600 ).title( "Face App" ) );
	settings->setFrameRate( 60.0f );
}

void FaceApp::setup()
{	
	gl::enableAlphaBlending();
	
	mFrameRate	= 0.0f;
	mFullScreen	= false;

	mDevice = Kinect2::Device::create();
	mDevice->enableFaceTracking3d();
	mDevice->start();
	mDevice->connectBodyEventHandler( [ & ]( const Kinect2::BodyFrame& frame )
	{
		mBodies = frame.getBodies();
	} );
	mDevice->connectBodyIndexEventHandler( [ & ]( const Kinect2::BodyIndexFrame frame )
	{
		mChannelBodyIndex = frame.getChannel();
	} );
	mDevice->connectDepthEventHandler( [ & ]( const Kinect2::DepthFrame frame )
	{
		mChannelDepth = frame.getChannel();
	} );
		
	mParams = params::InterfaceGl::create( "Params", Vec2i( 200, 100 ) );
	mParams->addParam( "Frame rate",	&mFrameRate,			"", true );
	mParams->addParam( "Full screen",	&mFullScreen ).key( "f" );
	mParams->addButton( "Quit",			[ & ]() { quit(); } ,	"key=q" );
}

void FaceApp::update()
{
	mFrameRate = getAverageFps();
	
	if ( mFullScreen != isFullScreen() ) {
		setFullScreen( mFullScreen );
		mFullScreen = isFullScreen();
	}
}

CINDER_APP_BASIC( FaceApp, RendererGl )
	