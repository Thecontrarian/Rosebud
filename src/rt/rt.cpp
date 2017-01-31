#include "rose.h"

static v8::Platform* static_platform;

void rose_init(const char* base_path) {
    v8::V8::InitializeICUDefaultLocation(base_path);
    v8::V8::InitializeExternalStartupData(base_path);
    static_platform = v8::platform::CreateDefaultPlatform();
    v8::V8::InitializePlatform(static_platform);
    v8::V8::Initialize();
}

void rose_deinit() {
    V8::Dispose();
    V8::ShutdownPlatform();
    delete static_platform;
}

void rose_rt::reset_palette_filter() {
    for (int i = 0; i < ROSE_PALETTE_INDEX_NUM; ++i) {
        palette_filter.begin[i] = (uint8_t) i;
    }
}

void rose_rt::reset_palette_transparency() {
    for (auto it = palette_transparency.begin; it != palette_transparency.end; it++) {
        *it = 0;
    }
    rose_set_bit(palette_transparency.begin, 0, true);
}

void rose_rt::reset_clipping_region() {
    int16_t* ptr = (int16_t*) clipping_region.begin;
    ptr[0] = 0;                      // x0
    ptr[1] = 0;                      // y0
    ptr[2] = (int16_t) ((meta.hd ? ROSE_HD_SCREEN_WIDTH : ROSE_SCREEN_WIDTH) - 1);  // x1
    ptr[3] = (int16_t) ((meta.hd ? ROSE_HD_SCREEN_HEIGHT : ROSE_SCREEN_HEIGHT) - 1); // y1
}

void rose_rt::reset_pen_color() {
    *pen_color_addr = 6;
}

void rose_rt::reset_print_cursor() {
    int16_t* ptr = (int16_t*) print_cursor.begin;
    ptr[0] = 0; // x0
    ptr[1] = 0; // y0 // TODO: replace with actual starting position
    // once font size is finalized
}

void rose_rt::reset_camera_offset() {
    int16_t* ptr = (int16_t*) camera_offset.begin;
    ptr[0] = 0; // x0
    ptr[1] = 0; // y0 // TODO: replace with actual starting position
    // once font size is finalized
}

void rose_rt::reset_pointer_positions() {
    int16_t* ptr = (int16_t*) pointer_positions.begin;
    ptr[0] = 0; // x0
    ptr[1] = 0; // y0 // TODO: replace with actual starting position

}

void rose_rt::reset_btn_states() {
    for (auto it = btn_states.begin; it != btn_states.end; it++) {
        *it = 0;
    }
    for (auto it = prev_btn_states.begin; it != prev_btn_states.end; it++) {
        *it = 0;
    }
}

void rose_rt::reset_mouse_wheel() {
    for (auto it = mouse_wheel.begin; it != mouse_wheel.end; it++) {
        *it = 0;
    }
}

void rose_rt::reset_key_states() {
    for (auto it = key_states.begin; it != key_states.end; it++) {
        *it = 0;
    }
    for (auto it = prev_key_states.begin; it != prev_key_states.end; it++) {
        *it = 0;
    }
}

void rose_rt::reset_palette() {
    memcpy(palette.begin, rose_default_palette, sizeof(rose_default_palette));
}

void rose_rt::reset_schema() {
    rose_api_graphics_set_spritesheet_meta(this, 0, 256, 256, 1, 1);
}

void rose_rt::reset_screen() {
    rose_api_graphics_cls(this);
}

void rose_rt::reset_userdata() {
    memset(mem->begin(), 0, (size_t) (meta.hd ? ROSE_HD_USERSPACE_MEMORY_SIZE : ROSE_USERSPACE_MEMORY_SIZE));
}

void rose_rt::copy_input_from_other(rose_rt* other) {
    auto len = prev_key_states.end - pointer_positions.begin;
    memcpy(pointer_positions.begin, other->pointer_positions.begin, len);
}


void rose_rt::copy_screen_from_other(rose_rt* other) {
    if (meta.hd == other->meta.hd) {
        memcpy(screen.begin, other->screen.begin, screen.end - screen.begin);
    } else if (meta.hd) {
        uint8_t* this_screen_ptr = screen.begin;
        uint8_t* other_screen_ptr = other->screen.begin;
        for (auto i = 0; i < ROSE_SCREEN_SIZE; i++) {
            this_screen_ptr[i*2] = other_screen_ptr[i];
            this_screen_ptr[i*2+1] = other_screen_ptr[i];
        }
    } else {
        uint8_t* this_screen_ptr = screen.begin;
        uint8_t* other_screen_ptr = other->screen.begin;
        for (auto i = 0; i < ROSE_SCREEN_SIZE; i++) {
            this_screen_ptr[i] = other_screen_ptr[i*2];
        }
    }
}

void rose_rt::make_mem_ranges() {
    bool hd = this->meta.hd;
    auto screen_begin = mem->end();
    std::advance(screen_begin, -(hd ? ROSE_HD_SCREEN_SIZE : ROSE_SCREEN_SIZE));
    screen.begin = screen_begin;
    screen.end = mem->end();

    auto it = screen.end;
    std::advance(it, -(hd ? ROSE_HD_RUNTIME_RESERVED_MEMORY_SIZE : ROSE_RUNTIME_RESERVED_MEMORY_SIZE));
    std::advance(it, -ROSE_PALETTE_SIZE);
    std::advance(it, -ROSE_MEMORY_SCHEMA_SIZE);
    schema.begin = it;
    std::advance(it, ROSE_MEMORY_SCHEMA_SIZE);
    schema.end = it;

    this->palette.begin = it;
    std::advance(it, ROSE_PALETTE_SIZE);
    this->palette.end = it;

    palette_filter.begin = it;
    std::advance(it, ROSE_PALETTE_INDEX_NUM);
    palette_filter.end = it;

    palette_transparency.begin = it;
    std::advance(it, ROSE_PALETTE_INDEX_NUM / 8);
    palette_transparency.end = it;

    clipping_region.begin = it;
    std::advance(it, 8);
    clipping_region.end = it;

    pen_color_addr = it;

    std::advance(it, 1);

    print_cursor.begin = it;
    std::advance(it, 4);
    print_cursor.end = it;

    camera_offset.begin = it;
    std::advance(it, 4);
    camera_offset.end = it;

    pointer_positions.begin = it;
    std::advance(it, 11 * 4 /* 2 16 bit number */);
    pointer_positions.end = it;

    btn_states.begin = it;
    std::advance(it, 4 /* 32 bit fields */);
    btn_states.end = it;

    prev_btn_states.begin = it;
    std::advance(it, 4 /* 32 bit fields */);
    prev_btn_states.end = it;

    mouse_wheel.begin = it;
    std::advance(it, 5 /* 2 16 bit ints and one bool */);
    mouse_wheel.end = it;

    key_states.begin = it;
    std::advance(it, 30 /* 240 bit fields */);
    key_states.end = it;

    prev_key_states.begin = it;
    std::advance(it, 30 /* 240 bit fields */);
    prev_key_states.end = it;
}

rose_rt::rose_rt(rose_fs* fs) {
    if (fs == NULL) {
        fprintf(stderr, "tried to create runtime base with null fs\n");
        exit(1);
    }

    this->meta.name = "";
    this->meta.author = "";
    this->meta.hd = false;
    this->fs = fs;

    this->mem = new std::array<uint8_t, ROSE_MEMORY_SIZE>();
    this->make_mem_ranges();
    this->mem->fill(0);
    reset_palette_filter();
    reset_palette_transparency();
    reset_clipping_region();
    reset_pen_color();
    reset_print_cursor();
    reset_camera_offset();
    reset_pointer_positions();
    reset_btn_states();
    reset_mouse_wheel();
    reset_key_states();
    reset_palette();
    reset_screen();
    reset_schema();
    reset_userdata();

    this->js = rose_js_create(this);
}

rose_rt::~rose_rt() {
    rose_js_free(this->js);
    // dont free fs, managed by system layer
    delete this->mem;
}

bool rose_rt::clear() {
    reset_palette_filter();
    reset_palette_transparency();
    reset_clipping_region();
    reset_pen_color();
    reset_print_cursor();
    reset_camera_offset();
    reset_palette();
    reset_schema();
    reset_userdata();
    this->js->module_cache.Reset();
    this->js->context.Reset();
    return true;
}


bool rose_rt::load_run_main() {
    if (this->self_cart == NULL) {
        return false;
    }

    auto isolate = this->js->isolate;

    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::TryCatch try_catch(isolate);

    Local<Context> context = Context::New(isolate, NULL, this->js->global_template.Get(isolate));

    rose_file* cart_info = rose_fs_fetch_cart_info_file(this->self_cart);
    if (cart_info != NULL) {
        if (!cart_info->in_mem) {
            auto err = this->fs->read_file(cart_info);
            if (err == ROSE_FS_CRITICAL_ERR) {
                return false;
            }
        }

        if (cart_info->buffer[cart_info->buffer_len-1] != '\0') {
            cart_info->buffer_len++;
            cart_info->buffer = (uint8_t*) realloc(cart_info->buffer, cart_info->buffer_len);
            cart_info->buffer[cart_info->buffer_len-1] = '\0';
            cart_info->last_modification = time(NULL);
        }


        v8::Local<v8::String> json;
        if (!v8::String::NewFromUtf8(isolate, (const char*) cart_info->buffer, v8::NewStringType::kNormal).ToLocal(&json)) {
            ReportException(isolate, &try_catch, this->error_cb);
            return false;
        }

        v8::Local<v8::Value> result;
        if (!v8::JSON::Parse(context, json).ToLocal(&result)) {
            ReportException(isolate, &try_catch, this->error_cb);
            return false;
        }
        if (result->IsObject()) {
            auto obj = result->ToObject();
            auto name_key = v8::String::NewFromUtf8(isolate, "name", v8::NewStringType::kNormal).ToLocalChecked();
            if (obj->Has(name_key)) {
                v8::String::Utf8Value name(obj->Get(name_key)->ToString());
                this->meta.name = *name;
            }

            auto author_key = v8::String::NewFromUtf8(isolate, "author", v8::NewStringType::kNormal).ToLocalChecked();
            if (obj->Has(author_key)) {
                v8::String::Utf8Value author(obj->Get(author_key)->ToString());
                this->meta.author = *author;
            }

            auto hd_key = v8::String::NewFromUtf8(isolate, "hd", v8::NewStringType::kNormal).ToLocalChecked();
            if (obj->Has(hd_key)) {
                this->meta.hd = obj->Get(hd_key)->ToBoolean()->BooleanValue();
            }
        }
    }

    make_mem_ranges();

//    this->mem->fill(0);
//    memcpy(this->palette->begin, rose_default_palette, sizeof(rose_default_palette));
//    rose_api_graphics_set_spritesheet_meta(this, 0, 256, 256, 1, 1);
//    auto data = rose_fs_fetch_cart_data_file(this->self_cart);
//    data->buffer = this->mem->begin();
//    data->buffer_len = this->palette->end - data->buffer;
//    this->fs->write_file(data);

    rose_file* cart_data = rose_fs_fetch_cart_data_file(this->self_cart);
    if (cart_data == NULL) {
        fprintf(stderr, "ERROR: no data file found\n");
        return false;
    }

    if (!cart_data->in_mem) {
        auto err = this->fs->read_file(cart_data);
        if (err == ROSE_FS_CRITICAL_ERR) {
            return false;
        }
    }

    if (cart_data->buffer_len > (this->mem->size() - (this->meta.hd ? ROSE_HD_RUNTIME_RESERVED_MEMORY_SIZE : ROSE_RUNTIME_RESERVED_MEMORY_SIZE))) {
        fprintf(stderr, "ERROR: tried to reload runtime and cartridge memory size was bigger than available memory size\n");
        return false;
    }
    memcpy(this->mem->data(), cart_data->buffer, cart_data->buffer_len);

    rose_file* main = rose_fs_fetch_cart_js_main(this->self_cart);
    if (main == NULL) {
        fprintf(stderr, "ERROR: no main file found\n");
        return false;
    }
    this->js->include_path.clear();
    this->js->include_path.push_back(main);

    if (!main->in_mem) {
        this->fs->read_file(main);
    }


    if (main->buffer[main->buffer_len-1] != '\0') {
        main->buffer_len++;
        main->buffer = (uint8_t*) realloc(main->buffer, main->buffer_len);
        main->buffer[main->buffer_len-1] = '\0';
        main->last_modification = time(NULL);
    }

    this->js->context.Reset(isolate, context);
    if (context.IsEmpty()) {
        fprintf(stderr, "something went horribly wrong and I'm so, so sorry.\n");
    }
    v8::Context::Scope context_scope(context);

    v8::Local<v8::String> file_name = v8::String::NewFromUtf8(isolate, main->name.c_str(), v8::NewStringType::kNormal).ToLocalChecked();
    v8::Local<v8::String> source;
    if (!v8::String::NewFromUtf8(isolate, (const char*) main->buffer, v8::NewStringType::kNormal).ToLocal(&source)) {
        ReportException(isolate, &try_catch, this->error_cb);
        return false;
    }

    Local<Map> module_cache = Map::New(isolate);
    this->js->module_cache.Reset(isolate, module_cache);

    bool failed;
    auto res = ExecuteString(isolate, source, file_name, true, &failed, this->error_cb);
    if (failed) {
        ReportException(isolate, &try_catch, this->error_cb);
        return false;
    }

    while (v8::platform::PumpMessageLoop(static_platform, isolate))
        continue;

    return true;
}

void rose_rt::save_input_frame() {
    memcpy(this->prev_btn_states.begin, this->btn_states.begin, this->prev_btn_states.end - this->prev_btn_states.begin);
    memcpy(this->prev_key_states.begin, this->key_states.begin, this->prev_key_states.end - this->prev_key_states.begin);
}

void rose_rt::update_mouse_pos(int16_t x, int16_t y) {
    int16_t* pointer = (int16_t*) this->pointer_positions.begin;
    pointer[20] = x;
    pointer[21] = y;
}

void rose_rt::update_btn_state(uint8_t btn, bool pressed) {
    rose_set_bit(this->btn_states.begin, btn, pressed);
}

void rose_rt::update_wheel_state(int16_t delta_x, int16_t delta_y, bool inverted) {
    int16_t* wheel_delta = (int16_t*) this->mouse_wheel.begin;
    wheel_delta[0] = delta_x;
    wheel_delta[1] = delta_y;

    bool* wheel_inverted = (bool*) (this->mouse_wheel.begin + 4);
    *wheel_inverted = inverted;
}

void rose_rt::update_keystate(rose_keycode keycode, bool pressed) {
    if (keycode < ROSE_KEYCODE_UNKNOWN) {
        rose_set_bit(this->key_states.begin, keycode, pressed);
    }
}

void rose_rt::retarget(rose_file* self, rose_file* target) {
    this->self_cart = self;
    this->target_cart = target;
}

bool rose_rt::rose_call( const char* name, uint8_t nargs, Local<Value>* args) {
    auto isolate = this->js->isolate;
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<Context> context = this->js->context.Get(isolate);
    if (context.IsEmpty()) {
        this->error_cb("Error: Something went wrong and I'm so sorry.\n");
        return false;
    }
    v8::Context::Scope context_scope(context);
    v8::TryCatch try_catch(isolate);
    Local<Object> jsGlobal = context->Global();
    Handle<Value> value = jsGlobal->Get(String::NewFromUtf8(isolate, name));
    Handle<Function> jsFunc = Handle<Function>::Cast(value);
    if (!jsFunc->IsFunction()) {
        return true;
    }
    v8::Local<v8::Value> result;
    if (!jsFunc->Call(context, jsGlobal, nargs, args).ToLocal(&result)) {
        assert(try_catch.HasCaught());
        ReportException(isolate, &try_catch, this->error_cb);
        return false;
    }
    assert(!try_catch.HasCaught());
    return true;
}

bool rose_rt::call_init() {
    return rose_call("_init", 0, NULL);
}

bool rose_rt::call_update() {
    return rose_call("_update", 0, NULL);
}

bool rose_rt::call_draw() {
    return rose_call("_draw", 0, NULL);
}

bool rose_rt::call_onmouse(int16_t x, int16_t y) {
    auto isolate = this->js->isolate;
    v8::HandleScope handle_scope(isolate);
    Local<Value> args[2] = {Int32::New(isolate,(int16_t) x), Int32::New(isolate,(int16_t) y)};
    return rose_call("_onmouse", 2, args);
}

bool rose_rt::call_onwheel(int16_t x, int16_t y, bool inverted) {
    auto isolate = this->js->isolate;
    v8::HandleScope handle_scope(isolate);
    Local<Value> args[3] = {Int32::New(isolate, x), Int32::New(isolate, y), Boolean::New(isolate, inverted)};
    return rose_call("_onwheel", 3, args);
}

bool rose_rt::call_onbtn(uint8_t code, bool pressed) {
    auto isolate = this->js->isolate;
    v8::HandleScope handle_scope(isolate);
    Local<Value> args[2] = {Int32::New(isolate, code), Boolean::New(isolate, pressed)};
    return rose_call("_onbtn", 2, args);
}

bool rose_rt::call_onkey(rose_keycode keycode, bool pressed, bool repeat) {
    auto isolate = this->js->isolate;
    v8::HandleScope handle_scope(isolate);
    Local<Value> args[3] = {Int32::New(isolate, keycode), Boolean::New(isolate, pressed), Boolean::New(isolate, repeat)};
    return rose_call("_onkey", 2, args);
}

bool rose_rt::call_ontouch() {
    // TODO: make this actually do something
    return rose_call("_ontouch", 0, NULL);
}


void rose_set_bit(std::array<uint8_t, ROSE_MEMORY_SIZE>::iterator arr, uint8_t addr, bool val) {
    uint8_t idx = (uint8_t) (addr / 8);
    uint8_t bit = (uint8_t) (addr % 8);
    if (val) {
        arr[idx] |= 1 << bit;
    } else {
        arr[idx] &= ~(1 << bit);
    }
}

bool rose_get_bit(std::array<uint8_t, ROSE_MEMORY_SIZE>::iterator arr, uint8_t addr) {
    uint8_t idx = (uint8_t) (addr / 8);
    uint8_t bit = (uint8_t) (addr % 8);
    return (bool) ((arr[idx] >> bit) & 1);
}


