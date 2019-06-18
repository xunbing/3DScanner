#pragma once
#include "afxwin.h"

#include <vtkResliceCursor.h>  
#include <vtkResliceCursorWidget.h>  

#include <vtkPlane.h>  
#include <vtkPlaneSource.h>  
#include <vtkPlaneWidget.h>  

#include <vtkImagePlaneWidget.h>  
#include <vtkResliceCursorThickLineRepresentation.h>  
#include <vtkResliceCursor.h>  

#include <vtkCommand.h>  
#include <vtkViewport.h>  
#include <vtkViewDependentErrorMetric.h>  

#include <vtkSmartPointer.h>  

#include <vtkRenderer.h>  
#include <vtkRendererSource.h>  

#include <vtkRenderingOpenGL2Module.h>
#include <vtkRenderWindow.h>  
#include <vtkWin32OpenGLRenderWindow.h>  

#include <vtkWin32RenderWindowInteractor.h> 

#include <vtkPolyVertex.h>
#include <vtkUnstructuredGrid.h>
#include <vtkDataSetMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>

#include <vtkLookupTable.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>

#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle)
VTK_MODULE_INIT(vtkRenderingFreeType)

using Pointf = struct _Pointf {
	float x;
	float y;
	float z;
	float r;
	float g;
	float b;
};

class CVtkViewer : public CStatic
{
	DECLARE_DYNAMIC(CVtkViewer)

public:
	CVtkViewer();
	virtual ~CVtkViewer();

	vtkSmartPointer<vtkActor> actor;
	void ReadPointCloud(std::vector<Pointf>&);

public:
	//3.2 重载CvtkView类PreSubclassWindow（）函数和OnPaint()函数  
	//PreSubclassWindow函数负责创建VTK可视化管线，OnPaint()函数负责客户区内场景渲染。  
	//vtkAcor,vtkRenderer,vtkRenderWindow,vtkRenderWindowInteractor四个部分。当然根据需要还可以设置vtkRenderWindowInteractorStyle,以及光照，材质，颜色等。  
	//在CvtkView类头文件中定义相关对象，并在PreSubclassWindow函数中实例化和构建可视化管线  
	void PreSubclassWindow();
	void SetImageData(vtkSmartPointer<vtkImageData> ImageData);
	void SetupReslice();
	void MoveWindow(CRect);

private:
	vtkSmartPointer< vtkImagePlaneWidget >   m_ImagePlaneWidget;
	vtkSmartPointer< vtkResliceCursorWidget> m_ResliceCursorWidget;
	vtkSmartPointer< vtkResliceCursor >      m_ResliceCursor;
	vtkSmartPointer< vtkResliceCursorThickLineRepresentation > m_ResliceCursorRep;

	vtkSmartPointer<vtkRenderer>         m_Renderer;
	vtkSmartPointer<vtkRenderWindow> m_RenderWindow;
	vtkSmartPointer<vtkImageData>       m_ImageData;

	//m_Direction为方向标志，取值分别为0,1和2，分别代表X轴，Y轴和Z轴方向，  
	int m_Direction;

protected:
	DECLARE_MESSAGE_MAP()
};
