/*
	*** Rendering View
	*** src/gui/viewer/render/view.h
	Copyright T. Youngs 2013-2019

	This file is part of Dissolve.

	Dissolve is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Dissolve is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Dissolve.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DISSOLVE_VIEW_H
#define DISSOLVE_VIEW_H

#include "gui/viewer/render/axes.h"
#include "gui/viewer/render/renderable.h"
#include "gui/viewer/render/renderablegroup.h"
#include "math/matrix4.h"
#include "templates/list.h"
#include "templates/reflist.h"

// Forward Declarations
class FontInstance;

// Rendering View
class View
{
	private:
	// Associated FontInstance from parent viewer
	FontInstance& fontInstance_;
	// List of Renderables that we are to display
	const List<Renderable>& renderables_;

	public:
	// Constructor / Destructor
	View(const List<Renderable>& renderables, FontInstance& fontInstance);
	~View();
	// Clear view, resetting to defaults
	void clear();


	/*
	 * Geometry
	 */
	private:
	// Pixel offsets for view
	int xOffset_, yOffset_;
	// Scale factors for view
	double xScale_, yScale_;
	// Aspect ratio of view
	double aspectRatio_;
	// Viewport matrix for GL
	GLuint viewportMatrix_[4];
	// Version of viewport matrix
	int viewportVersion_;
	

	public:
	// Set pixel offsets and scales to use
	void setOffsetAndScale(int xOffset, int yOffset, double xScale, double yScale);
	// Recalculate viewport matrix based on pixel dimensions provided
	void recalculateViewport(int width, int height);
	// Translate viewport by specified pixel amounts
	void translateViewport(int deltaX, int deltaY);
	// Return viewport matrix
	const GLuint* viewportMatrix() const;


	/*
	 * Projection / View
	 */
	public:
	// View type
	enum ViewType { NormalView, AutoStretchedView, FlatXYView, FlatXZView, FlatZYView, LinkedView, nViewTypes };
	// Convert text string to ViewType
	static ViewType viewType(const char* s);
	// Convert ViewType to text string
	static const char* viewType(ViewType vt);
	// AutoFollow type
	enum AutoFollowType { NoAutoFollow, AllAutoFollow, XAutoFollow, nAutoFollowTypes };
	// Convert text string to AutoFollowType
	static AutoFollowType autoFollowType(const char* s);
	// Convert AutoFollowType to text string
	static const char* autoFollowType(AutoFollowType aft);

	private:
	// Type of view to use
	ViewType viewType_;
	// Projection matrix for GL
	Matrix4 projectionMatrix_;
	// Whether projection has perspective
	bool hasPerspective_;
	// Field of view angle used in projectionMatrix_ when using perspective
	double perspectiveFieldOfView_;
	// View rotation matrix for GL
	Matrix4 viewRotation_;
	// Logpoint for view rotation matrix
	int viewRotationPoint_;
	// View rotation matrix inverse
	Matrix4 viewRotationInverse_;
	// Logpoint for view rotation inverse matrix calculation
	int viewRotationInversePoint_;
	// Current translation of view
	Vec3<double> viewTranslation_;
	// Full view matrix (rotation + translation)
	Matrix4 viewMatrix_;
	// Inverse of view matrix
	Matrix4 viewMatrixInverse_;
	// Standard zOffset for translation matrix
	static const double zOffset_;
	// Axes version at which view matrix was last calculated (mostly for keeping 2D view correct)
	int viewAxesUsedAt_;
	// Viewport version at which view matrix was last calculated
	int viewViewportUsedAt_;
	// Auto-follow type in effect
	AutoFollowType autoFollowType_;
	// Length of X region to follow, if autoFollowType_ == XFollow
	double autoFollowXLength_;

	private:
	// Return calculated projection matrix
	Matrix4 calculateProjectionMatrix(bool hasPerspective, double orthoZoom = 0.0) const;

	public:
	// Set view type
	void setViewType(View::ViewType vt);
	// Return view type
	View::ViewType viewType() const;
	// Return whether view type is flat
	bool isFlatView() const;
	// Return projection matrix
	Matrix4 projectionMatrix() const;
	// Set whether the view uses perspective
	void setHasPerspective(bool perspective);
	// Return whether the view uses perspective
	bool hasPerspective() const;
	// Update view matrix
	void setViewRotation(Matrix4& mat);
	// Update single column of view matrix
	void setViewRotationColumn(int column, double x, double y, double z);
	// Rotate view matrix about x and y by amounts specified
	void rotateView(double dx, double dy);
	// Return view rotation
	Matrix4 viewRotation() const;
	// Return view rotation inverse
	Matrix4 viewRotationInverse();
	// Set view translation
	void setViewTranslation(double x, double y, double z);
	// Translate view matrix by amounts specified
	void translateView(double dx, double dy, double dz);
	// Return current view translation
	Vec3<double> viewTranslation() const;
	// Update view matrix
	void updateViewMatrix();
	// Return full view matrix (rotation + translation)
	const Matrix4& viewMatrix() const;
	// Project given data coordinates into world coordinates
	Vec3<double> dataToWorld(Vec3<double> r) const;
	// Project given data coordinates into screen coordinates
	Vec3<double> dataToScreen(Vec3<double> r) const;
	// Project given data coordinates into screen coordinates, with corresponding distance 'delta' in data
	Vec3<double> dataToScreen(Vec3<double> r, double& delta) const;
	// Project given data coordinates into screen coordinates using supplied rotation matrix and translation vector
	Vec3<double> dataToScreen(Vec3<double> r, Matrix4 projectionMatrix, Matrix4 rotationMatrix, Vec3<double> translation = Vec3<double>()) const;
	// Return z translation necessary to display coordinates supplied, assuming the identity view matrix
	double calculateRequiredZoom(double xMax, double yMax, double fraction) const;
	// Convert screen coordinates into data space coordinates
	Vec3<double> screenToData(int x, int y, double z) const;
	// Calculate selection axis coordinate from supplied screen coordinates
	double screenToAxis(int axis, int x, int y, bool clamp) const;
	// Recalculate current view parameters (e.g. for 2D, autostretched 3D etc.)
	void recalculateView(bool force = false);
	// Reset view matrix to face XY plane
	void resetViewMatrix();
	// Set display limits to show all available data
	void showAllData(double xFrac = 1.0, double yFrac = 1.0, double zFrac = 1.0);
	// Zoom to specified region
	void zoomTo(Vec3<double> limit1, Vec3<double> limit2);
	// Set auto-follow type in effect
	void setAutoFollowType(AutoFollowType aft);
	// Cycle auto-follow type in effect
	void cycleAutoFollowType();
	// Return auto-follow type in effect
	AutoFollowType autoFollowType() const;
	// Set length of X region to follow, if autoFollowType_ == XFollow
	void setAutoFollowXLength(double length);
	// Return length of X region to follow, if autoFollowType_ == XFollow
	double autoFollowXLength() const;
	// Set axis limits based on current auto-follow type
	void autoFollowData();


	/*
	 * Axes
	 */
	private:
	// Axes for the view
	Axes axes_;
	// Pixel 'lengths' of axes in flat views
	Vec3<double> axisPixelLength_;

	public:
	// Return absolute minimum transformed values over all associated collections
	Vec3<double> transformedDataMinima();
	// Return absolute maximum transformed values over all associated collections
	Vec3<double> transformedDataMaxima();
	// Return absolute minimum positive transformed values over all associated collections
	Vec3<double> transformedDataPositiveMinima();
	// Return absolute maximum positive transformed values over all associated collections
	Vec3<double> transformedDataPositiveMaxima();
	// Update axis limits to represent data extent of associated collections
	void updateAxisLimits(double xFrac =1.0, double yFrac = 1.0, double zFrac = 1.0);
	// Shift flat view axis limits by specified amounts
	void shiftFlatAxisLimits(double deltaH, double deltaV);
	// Shift flat view axis limits by specified fractional amounts
	void shiftFlatAxisLimitsFractional(double fracH, double fracV);
	// Return axes for the view
	Axes& axes();
	// Return axes for the view (const)
	const Axes& constAxes() const;


	/*
	 * Style
	 */
	private:
	// Font scaling for axis value labels
	double labelPointSize_;
	// Font scaling for titles
	double titlePointSize_;
	// Text z scaling factor
	double textZScale_;
	// Whether axis text labels are drawn flat in 3D views
	bool flatLabelsIn3D_;

	private:
	// Calculate font scaling factor
	void calculateFontScaling();

	public:
	// Set font point size for axis value labels
	void setLabelPointSize(double value);
	// Return font point size for axis value labels
	double labelPointSize();
	// Return font point size for titles
	void setTitlePointSize(double value);
	// Return font point size for titles
	double titlePointSize();
	// Return text z scaling factor
	double textZScale();
	// Set whether axis text labels are drawn flat in 3D views
	void setFlatLabelsIn3D(bool flat);
	// Whether axis text labels are drawn flat in 3D views
	bool flatLabelsIn3D();


	/*
	 * Interaction
	 */
	public:
	// Return axis title at specified coordinates (if any)
	int axisTitleAt(int screenX, int screenY);
};

#endif