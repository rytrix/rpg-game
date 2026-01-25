#pragma once

class NoCopy {
public:
    NoCopy() = default;
    ~NoCopy() = default;

    NoCopy(const NoCopy&) = delete;
    NoCopy& operator=(const NoCopy&) = delete;
    NoCopy(NoCopy&& other) noexcept = default;
    NoCopy& operator=(NoCopy&& other) noexcept = default;
};

class NoCopyNoMove {
public:
    NoCopyNoMove() = default;
    ~NoCopyNoMove() = default;

    NoCopyNoMove(const NoCopyNoMove&) = delete;
    NoCopyNoMove& operator=(const NoCopyNoMove&) = delete;
    NoCopyNoMove(NoCopyNoMove&& other) noexcept = delete;
    NoCopyNoMove& operator=(NoCopyNoMove&& other) noexcept = delete;
};