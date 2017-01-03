//
// Created by Dan Lewis on 12/31/16.
//

#ifndef ROSEBUD_JS_GRAPHICS_H
#define ROSEBUD_JS_GRAPHICS_H

#include "config.h"
#include "rt/api/api_graphics.h"

void rose_js_graphics_pset(const v8::FunctionCallbackInfo<v8::Value>& args);

void rose_js_graphics_pget(const v8::FunctionCallbackInfo<v8::Value>& args);

void rose_js_graphics_palset(const v8::FunctionCallbackInfo<v8::Value>& args);

void rose_js_graphics_palget(const v8::FunctionCallbackInfo<v8::Value>& args);

void rose_js_graphics_line(const v8::FunctionCallbackInfo<v8::Value>& args);

void rose_js_graphics_rect(const v8::FunctionCallbackInfo<v8::Value>& args);

void rose_js_graphics_rectfill(const v8::FunctionCallbackInfo<v8::Value>& args);

void rose_js_graphics_circ(const v8::FunctionCallbackInfo<v8::Value>& args);

void rose_js_graphics_circfill(const v8::FunctionCallbackInfo<v8::Value>& args);

void rose_js_graphics_tri(const v8::FunctionCallbackInfo<v8::Value>& args);

void rose_js_graphics_trifill(const v8::FunctionCallbackInfo<v8::Value>& args);

void rose_js_graphics_cls(const v8::FunctionCallbackInfo<v8::Value>& args);

#endif //ROSEBUD_JS_GRAPHICS_H
