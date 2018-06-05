
#include "Composer.h"

namespace android {

void Composer::openGlobalTransactionImpl() {
    { // scope for the lock
        Mutex::Autolock _l(mLock);
        mTransactionNestCount += 1;
    }
}

void Composer::closeGlobalTransactionImpl(bool synchronous) {
    sp<ISurfaceComposer> sm(ComposerService::getComposerService());

    Vector<ComposerState> transaction;
    Vector<DisplayState> displayTransaction;
    uint32_t flags = 0;

    { // scope for the lock
        Mutex::Autolock _l(mLock);
        mForceSynchronous |= synchronous;
        if (!mTransactionNestCount) {
            ALOGW("At least one call to closeGlobalTransaction() was not matched by a prior "
                    "call to openGlobalTransaction().");
        } else if (--mTransactionNestCount) {
            return;
        }

        transaction = mComposerStates;
        mComposerStates.clear();

        displayTransaction = mDisplayStates;
        mDisplayStates.clear();

        if (mForceSynchronous) {
            flags |= ISurfaceComposer::eSynchronous;
        }
        if (mAnimation) {
            flags |= ISurfaceComposer::eAnimation;
        }

        mForceSynchronous = false;
        mAnimation = false;
    }

   sm->setTransactionState(transaction, displayTransaction, flags);
}

void Composer::setAnimationTransactionImpl() {
    Mutex::Autolock _l(mLock);
    mAnimation = true;
}

layer_state_t * Composer::getLayerStateLocked(const sp<SurfaceComposerClient> & client, const sp<IBinder> & id) {

    ComposerState s;
    s.client = client->mClient;
    s.state.surface = id;

    ssize_t index = mComposerStates.indexOf(s);
    if (index < 0) {
        // we don't have it, add an initialized layer_state to our list
        index = mComposerStates.add(s);
    }

    ComposerState* const out = mComposerStates.editArray();
    return &(out[index].state);
}

// ---------------------------------------------------------------------------

DisplayState & Composer::getDisplayStateLocked(const sp<IBinder> & token) {
    DisplayState s;
    s.token = token;
    ssize_t index = mDisplayStates.indexOf(s);
    if (index < 0) {
        // we don't have it, add an initialized layer_state to our list
        s.what = 0;
        index = mDisplayStates.add(s);
    }
    return mDisplayStates.editItemAt(index);
}

// ---------------------------------------------------------------------------

sp<IBinder> Composer::createDisplay(const String8 & displayName, bool secure) {
    return ComposerService::getComposerService()->createDisplay(displayName,
            secure);
}

void Composer::destroyDisplay(const sp<IBinder> & display) {
    return ComposerService::getComposerService()->destroyDisplay(display);
}

sp<IBinder> Composer::getBuiltInDisplay(int32_t id) {
    return ComposerService::getComposerService()->getBuiltInDisplay(id);
}

status_t Composer::setPosition(const sp<SurfaceComposerClient> & client, const sp<IBinder> & id, float x, float y) {
    Mutex::Autolock _l(mLock);
    layer_state_t* s = getLayerStateLocked(client, id);
    if (!s)
        return BAD_INDEX;
    s->what |= layer_state_t::ePositionChanged;
    s->x = x;
    s->y = y;
    return NO_ERROR;
}

status_t Composer::setSize(const sp<SurfaceComposerClient> & client, const sp<IBinder> & id, uint32_t w, uint32_t h) {
    Mutex::Autolock _l(mLock);
    layer_state_t* s = getLayerStateLocked(client, id);
    if (!s)
        return BAD_INDEX;
    s->what |= layer_state_t::eSizeChanged;
    s->w = w;
    s->h = h;

    // Resizing a surface makes the transaction synchronous.
    mForceSynchronous = true;

    return NO_ERROR;
}

status_t Composer::setLayer(const sp<SurfaceComposerClient> & client, const sp<IBinder> & id, int32_t z) {
    Mutex::Autolock _l(mLock);
    layer_state_t* s = getLayerStateLocked(client, id);
    if (!s)
        return BAD_INDEX;
    s->what |= layer_state_t::eLayerChanged;
    s->z = z;
    return NO_ERROR;
}

status_t Composer::setFlags(const sp<SurfaceComposerClient> & client, const sp<IBinder> & id, uint32_t flags, uint32_t mask) {
    Mutex::Autolock _l(mLock);
    layer_state_t* s = getLayerStateLocked(client, id);
    if (!s)
        return BAD_INDEX;
    if (mask & layer_state_t::eLayerOpaque) {
        s->what |= layer_state_t::eOpacityChanged;
    }
    if (mask & layer_state_t::eLayerHidden) {
        s->what |= layer_state_t::eVisibilityChanged;
    }
    s->flags &= ~mask;
    s->flags |= (flags & mask);
    s->mask |= mask;
    return NO_ERROR;
}

status_t Composer::setTransparentRegionHint(const sp<SurfaceComposerClient> & client, const sp<IBinder> & id, const Region & transparentRegion) {
    Mutex::Autolock _l(mLock);
    layer_state_t* s = getLayerStateLocked(client, id);
    if (!s)
        return BAD_INDEX;
    s->what |= layer_state_t::eTransparentRegionChanged;
    s->transparentRegion = transparentRegion;
    return NO_ERROR;
}

status_t Composer::setAlpha(const sp<SurfaceComposerClient> & client, const sp<IBinder> & id, float alpha) {
    Mutex::Autolock _l(mLock);
    layer_state_t* s = getLayerStateLocked(client, id);
    if (!s)
        return BAD_INDEX;
    s->what |= layer_state_t::eAlphaChanged;
    s->alpha = alpha;
    return NO_ERROR;
}

status_t Composer::setMatrix(const sp<SurfaceComposerClient> & client, const sp<IBinder> & id, float dsdx, float dtdx, float dsdy, float dtdy) {
    Mutex::Autolock _l(mLock);
    layer_state_t* s = getLayerStateLocked(client, id);
    if (!s)
        return BAD_INDEX;
    s->what |= layer_state_t::eMatrixChanged;
    layer_state_t::matrix22_t matrix;
    matrix.dsdx = dsdx;
    matrix.dtdx = dtdx;
    matrix.dsdy = dsdy;
    matrix.dtdy = dtdy;
    s->matrix = matrix;
    return NO_ERROR;
}

status_t Composer::setOrientation(int orientation) {
}

status_t Composer::setCrop(const sp<SurfaceComposerClient> & client, const sp<IBinder> & id, const Rect & crop) {
    Mutex::Autolock _l(mLock);
    layer_state_t* s = getLayerStateLocked(client, id);
    if (!s)
        return BAD_INDEX;
    s->what |= layer_state_t::eCropChanged;
    s->crop = crop;
    return NO_ERROR;
}

status_t Composer::setLayerStack(const sp<SurfaceComposerClient> & client, const sp<IBinder> & id, uint32_t layerStack) {
    Mutex::Autolock _l(mLock);
    layer_state_t* s = getLayerStateLocked(client, id);
    if (!s)
        return BAD_INDEX;
    s->what |= layer_state_t::eLayerStackChanged;
    s->layerStack = layerStack;
    return NO_ERROR;
}

void Composer::setDisplaySurface(const sp<IBinder> & token, const sp<IGraphicBufferProducer> & bufferProducer) {
    Mutex::Autolock _l(mLock);
    DisplayState& s(getDisplayStateLocked(token));
    s.surface = bufferProducer;
    s.what |= DisplayState::eSurfaceChanged;
}

void Composer::setDisplayLayerStack(const sp<IBinder> & token, uint32_t layerStack) {
    Mutex::Autolock _l(mLock);
    DisplayState& s(getDisplayStateLocked(token));
    s.layerStack = layerStack;
    s.what |= DisplayState::eLayerStackChanged;
}

void Composer::setDisplayProjection(const sp<IBinder> & token, uint32_t orientation, const Rect & layerStackRect, const Rect & displayRect) {
    Mutex::Autolock _l(mLock);
    DisplayState& s(getDisplayStateLocked(token));
    s.orientation = orientation;
    s.viewport = layerStackRect;
    s.frame = displayRect;
    s.what |= DisplayState::eDisplayProjectionChanged;
    mForceSynchronous = true; // TODO: do we actually still need this?
}

void Composer::setDisplaySize(const sp<IBinder> & token, uint32_t width, uint32_t height) {
    Mutex::Autolock _l(mLock);
    DisplayState& s(getDisplayStateLocked(token));
    s.width = width;
    s.height = height;
    s.what |= DisplayState::eDisplaySizeChanged;
}


} // namespace android
